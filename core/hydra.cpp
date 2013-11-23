#include "hydra.h"
#include <Arduino.h>

// HydraComponent
/////////////////

const char* HydraComponent::name = "Nil";

const HydraConfigValueDescriptionList HydraComponent::config_value_description_list = {
	0, 0, 0
};

const char* HydraComponent::getName() {
	return HydraComponent::name;
}

const HydraConfigValueDescriptionList* HydraComponent::getConfigDescription() {
	return & config_value_description_list;
}

uint8_t* HydraComponent::getConfig() {
	return 0;
}

void HydraComponent::init(Hydra* hydra) {
	hydra_debug("HydraComponent::init begin");
	this->hydra = hydra;
	hydra_debug("HydraComponent::init end");
}

bool HydraComponent::writePacket(const HydraPacket* packet) {
	return false;
}

bool HydraComponent::isPacketAvailable() {
	return false;
}

bool HydraComponent::readPacket(HydraPacket* packet) {
	return false;
}


// HydraNetComponent
////////////////////

bool HydraNetComponent::writePacket(const HydraPacket* packet) {
	HydraAddress gateway = this->getGateway(packet->part.to_addr);
	if ((gateway.raw != HYDRA_ADDR_NULL) && (gateway.raw != HYDRA_ADDR_LOCAL)) {
		return this->sendPacket(packet->part.to_addr, packet, hydra_is_addr_local(packet->part.from_addr));
	} else {
		return false;
	}
}

HydraAddress HydraNetComponent::getGateway(HydraAddress destionation) {
	destionation.raw = HYDRA_ADDR_NULL;
	return destionation;
}

bool HydraNetComponent::sendPacket(const HydraAddress to, const HydraPacket* packet, const bool set_from_addr) {
	return false;
}

HydraAddress HydraNetComponent::getAddress() {
	return {HYDRA_ADDR_NULL};
}

// Hydra
////////

Hydra::Hydra(HydraComponentDescriptionList* components) {
	this->components = components;
	this->ms = millis();
	this->timestamp = 0;
	this->timestamp_last = 0;
	this->default_gateway.raw = HYDRA_ADDR_NULL;
}

void Hydra::bootConsole() {
	Serial.print("\nSend any to enter Hydra configuration console.");
	byte countdown = HYDRA_BOOT_CONSOLE_WAIT_TIME * 10;
	pinMode(13, OUTPUT);
	while (countdown--) {
		delay(100);
		if (Serial.available() > 0) {
			while(Serial.available() > 0) {
				Serial.read();
			}
			Serial.print("\n");
			this->consoleRun();
			digitalWrite(13, LOW);
			return;
		}
		Serial.print(".");
		digitalWrite(13, countdown & 1 ? HIGH : LOW);
	}
	Serial.print("\n");
	digitalWrite(13, LOW);
}

void Hydra::consoleRun() {
	Serial.println("Welcome to Hydra configuration console!");
	this->consoleHelp();
	while(true) {
		if (Serial.available()) {
			String input = Serial.readString();
			switch(input[0]) {
				case 'q':
					return;
				case 'h':
					this->consoleHelp();
					break;
				case 'p':
					this->consolePrintConfig();
					break;
				case 'r':
					this->loadConfig();
					Serial.println("Read from EEPROM");
					break;
				case 'w':
					this->saveConfig();
					Serial.println("Write to EEPROM");
					break;
				case 's':
					{
						int pos = input.indexOf('=');
						String name = input.substring(2, pos);
						name.trim();
						String value = input.substring(pos + 1);
						value.trim();
						if (this->consoleSetConfigValue(name, value)) {
							Serial.println("Set value successful");
						}
					}
					break;
				case 'd':
					{
						int i = 0;
						while(i < 2048) {
							this->consolePrintHex((i >> 8) & 0xff);
							this->consolePrintHex(i & 0xff);
							Serial.print(' ');
							this->consolePrintHex((uint8_t *) i, 32);
							Serial.print('\n');
							i += 32;
						}
					}
					break;
				default:
					Serial.println("Unknown command");
			}
		}
		digitalWrite(13, millis() & 0x400 ? HIGH : LOW);
	}
}

void Hydra::consoleHelp() {
	Serial.println("Commands:");
	Serial.println("q - Quit");
	Serial.println("h - Help");
	Serial.println("p - Print current values");
	Serial.println("r - Read from EEPROM");
	Serial.println("w - Write to EEPROM");
	Serial.println("s param=hex - Set value, like in 'p'");
}


void Hydra::consolePrintHex(uint8_t value) {
	if (value < 0x10) {
		Serial.print('0');
	}
	Serial.print(value, HEX);
}

void Hydra::consolePrintHex(uint8_t *buffer, int length) {
	int i = 0;
	while (i < length) {
		this->consolePrintHex(buffer[i]);
		i++;
	}
}

String Hydra::consoleGetConfigValueFullName(uint8_t index, uint8_t value_index) {
	String name;
	HydraComponentDescription* item = & this->components->list[index];
	name = item->component->getName();
	name.concat("[");
	name.concat(index);
	name.concat("].");
	name.concat(item->component->getConfigDescription()->list[value_index].caption);
	return name;
}

void Hydra::consolePrintConfig() {
	int i, j;
	for(i = 0; i < this->components->totalCount; ++i) {
		HydraComponentDescription* item = & this->components->list[i];
		const HydraConfigValueDescriptionList* config_description = item->component->getConfigDescription();
		int offset = 0;
		for(j = 0; j < config_description->count; ++j) {
			String name = this->consoleGetConfigValueFullName(i, j);
			Serial.print(name);
			Serial.print('=');
			uint8_t* config = item->component->getConfig();
			this->consolePrintHex(config + offset, config_description->list[j].size);
			Serial.print('\n');
			offset += config_description->list[j].size;
		}
	}
}

bool Hydra::consoleParseHex(String hex, uint8_t* buffer, uint8_t length) {
	if (hex.length() != length * 2) {
		Serial.println("Wrong value length");
		return false;
	}

	uint8_t tmp[length];
	unsigned int i;

	for(i = 0; i < hex.length(); ++i) {
		char c = hex.charAt(i);
		byte b = 0;
		if ((c >= '0') && (c <= '9')) {
			b = c - '0';
		} else if ((c >= 'A') && (c <= 'F')) {
			b = c - 'A' + 0x0A;
		} else if ((c >= 'a') && (c <= 'f')) {
			b = c - 'a' + 0x0A;
		} else {
			Serial.print("Wrong value symbol at ");
			Serial.print(i);
			Serial.print(" '");
			Serial.print(c);
			Serial.println("'.");
			return false;
		}
		if (i & 1) {
			//lo
			tmp[i >> 1] |= b;
		} else {
			//hi
			tmp[i >> 1] = b << 4;
		}
	}
	memcpy(buffer, tmp, length);
	return true;
}


bool Hydra::consoleSetConfigValue(String name, String value) {
	int i, j;
	for(i = 0; i < this->components->totalCount; ++i) {
		HydraComponentDescription* item = & this->components->list[i];
		const HydraConfigValueDescriptionList* config_description = item->component->getConfigDescription();
		int offset = 0;
		for(j = 0; j < config_description->count; ++j) {
			String pname = this->consoleGetConfigValueFullName(i, j);
			if (pname == name) {
				return this->consoleParseHex(value, item->component->getConfig() + offset, config_description->list[j].size);
			}
			offset += config_description->list[j].size;
		}
	}
	Serial.println("Param not found");
	return false;
}

void Hydra::loadConfig() {
	hydra_debug("Hydra::loadConfig begin");
	int i, j;
	int offset = 0;
	for(i = 0; i < this->components->totalCount; ++i) {
		HydraComponentDescription* item = & this->components->list[i];
		const HydraConfigValueDescriptionList* config_description = item->component->getConfigDescription();
		uint8_t* config = item->component->getConfig();
		for(j = 0; j < config_description->size; ++j) {
			config[j] = EEPROM.read(offset);
			++offset;
		}
	}
	hydra_debug("Hydra::loadConfig end");
}

void Hydra::saveConfig() {
	hydra_debug("Hydra::saveConfig begin");
	int i, j;
	int offset = 0;
	for(i = 0; i < this->components->totalCount; ++i) {
		HydraComponentDescription* item = & this->components->list[i];
		const HydraConfigValueDescriptionList* config_description = item->component->getConfigDescription();
		for(j = 0; j < config_description->size; ++j) {
			uint8_t* config = item->component->getConfig();
			if (config[j] != EEPROM.read(offset)) {
				EEPROM.write(offset, config[j]);
			}
			++offset;
		}
	}
	hydra_debug("Hydra::saveConfig end");
}

void Hydra::init() {
	hydra_debug("Hydra::init begin");
	Serial.println("# Hydra wakes #");
	this->loadConfig();
	this->bootConsole();

	Serial.println("# Hydra awake #");
	int i;
	for(i = 0; i < this->components->totalCount; ++i) {
		hydra_debug_param("Hydra::init component ", i);
		this->components->list[i].component->init(this);
	}
	Serial.println("# Hydra is ready #");
	hydra_debug("Hydra::init end");
}

void Hydra::updateTimer() {
	uint32_t ms = millis();
	uint32_t delta_ms = ms - this->ms;
	this->timestamp += delta_ms / 1000;
	this->ms = ms - delta_ms % 1000;
}

void Hydra::loop() {
	//hydra_debug("Hydra::loop begin");
	int i;
	HydraPacket packet;

	// enumerate services
	for(i = 0; i < this->components->totalCount; ++i) {
		this->updateTimer();

		HydraComponent* item = this->components->list[i].service.component;
		if (item->isPacketAvailable()) {
			memset(& packet, 0, HYDRA_PACKET_SIZE);
			if (item->readPacket(& packet)) {
				HydraAddress received_via;
				if (i < this->components->netifCount) {
					received_via.raw = ((HydraNetComponent*) item)->getAddress().raw;
				} else {
					received_via.raw = HYDRA_ADDR_LOCAL;
					packet.part.from_service = this->components->list[i].id;
					packet.part.timestamp = this->getTime();
				}
				this->route(& packet, received_via);
			}
		}
	}
	//hydra_debug("Hydra::loop end");
}

void Hydra::route(const HydraPacket* packet, const HydraAddress received_via) {
	hydra_debug("Hydra::route begin");
	hydra_debug_param("Hydra::route packet from_addr ", packet->part.from_addr.raw);
	hydra_debug_param("Hydra::route packet to_addr ", packet->part.to_addr.raw);
	hydra_debug_param("Hydra::route packet from_service ", packet->part.from_service);
	hydra_debug_param("Hydra::route packet to_service ", packet->part.to_service);
	hydra_debug_param("Hydra::route packet timestamp ", packet->part.timestamp);
	#ifdef HYDRA_DEBUG
		Serial.print("Packet raw: ");
		this->consolePrintHex((uint8_t*)packet, HYDRA_PACKET_SIZE);
		Serial.print('\n');
	#endif
	int i;

	HydraAddress destionation = packet->part.to_addr;
	bool need_set_from_addr = (hydra_is_addr_local(received_via) && hydra_is_addr_null(packet->part.from_addr));
	if (hydra_is_addr_null(destionation)) {
		//drop
	} else if (hydra_is_addr_local(destionation)) {
		this->landing(packet, received_via, {HYDRA_ADDR_LOCAL});
	} else if (hydra_is_addr_to_all(destionation)) {
		//send to all nets (exclude source)
		for(i = 0; i < this->components->netifCount; ++i) {
			HydraNetComponent* item = this->components->list[i].netif.component;
			if (received_via.raw != item->getAddress().raw) {
				item->sendPacket(destionation, packet, need_set_from_addr);
			}
		}
		//and to me
		this->landing(packet, received_via, received_via);

	} else if (hydra_is_addr_to_net(destionation)) {
		//send to exactly to net (exclude source)
		for(i = 0; i < this->components->netifCount; ++i) {
			HydraNetComponent* item = this->components->list[i].netif.component;
			HydraAddress gateway = item->getGateway(packet->part.to_addr);
			if (gateway.raw != HYDRA_ADDR_NULL) {
				if (received_via.raw != item->getAddress().raw) {
					item->sendPacket(destionation, packet, need_set_from_addr);
				}
				//and to me
				this->landing(packet, received_via, item->getAddress());
				break;
			}
		}

	} else if (hydra_is_addr_to_devices(destionation)) {
		//send to all nets (exclude source)
		bool landed = false;
		for(i = 0; i < this->components->netifCount; ++i) {
			HydraNetComponent* item = this->components->list[i].netif.component;
			if (received_via.raw != item->getAddress().raw) {
				item->sendPacket(destionation, packet, need_set_from_addr);
			}
			if (!landed && (destionation.part.device == item->getAddress().part.device)) {
				//and me
				landed = true;
				this->landing(packet, received_via, item->getAddress());
			}
		}

	} else {
		for(i = 0; i < this->components->netifCount; ++i) {
			HydraNetComponent* item = this->components->list[i].netif.component;
			HydraAddress gateway = item->getGateway(packet->part.to_addr);
			if (gateway.raw == HYDRA_ADDR_LOCAL) {
				this->landing(packet, received_via, item->getAddress());
				break;
			} else if (gateway.raw != HYDRA_ADDR_NULL) {
				if (received_via.raw != item->getAddress().raw) {
					item->sendPacket(gateway, packet, need_set_from_addr);
					break;
				}
			}
		}

	}
	hydra_debug("Hydra::route end");
}

void Hydra::landing(const HydraPacket* packet, const HydraAddress received_via, const HydraAddress landing_via) {
	hydra_debug("Hydra::landing begin");
	int i;
	HydraPacket p;
	memcpy(&p, packet, HYDRA_PACKET_SIZE);
	p.part.to_addr = landing_via;
	for(i = this->components->netifCount; i < this->components->totalCount; ++i) {
		HydraComponentDescription* item = & this->components->list[i];
		if (item->service.id == p.part.to_service) {
			hydra_debug_param("Hydra::landing service ", item->service.id);
			hydra_debug_param("Hydra::landing component ", item->component->getName());
			item->component->writePacket(& p);
		}
	}
	hydra_debug("Hydra::landing end");
}

uint32_t Hydra::getTime() {
	this->updateTimer();
	return this->timestamp;
}

int32_t Hydra::getTimeZoneOffset() {
	return this->timezone_offset_seconds;
}

bool Hydra::isTimeSynced() {
	return this->timestamp_last ? abs(this->timestamp - this->timestamp_last) < 600 : false;
}

void Hydra::setTime(uint32_t timestamp, int16_t timezone_offset_minutes) {
	this->timestamp = timestamp;
	this->timestamp_last = timestamp;
	if ((timezone_offset_minutes <= 720) || (timezone_offset_minutes >= -720)) {
		this->timezone_offset_seconds = timezone_offset_minutes;
		this->timezone_offset_seconds *= 60;
	}
}

HydraAddress Hydra::getDefaultGateway() {
	return this->default_gateway;
}
