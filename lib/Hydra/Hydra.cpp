#include "Hydra.h"
#include "HydraEcho.h"
#include <EEPROM.h>

// HydraComponent
/////////////////

/* Example
const char* HydraComponent::name = "Nil";
*/

/* Example
const HydraConfigValueDescriptionList HydraComponent::config_value_description_list = {
    0, 0, 0
};
*/

const char* HydraComponent::getName() {
    /* Example
    return HydraComponent::name;
    */
    return 0;
}

const HydraConfigValueDescriptionList* HydraComponent::getConfigDescription() {
    /* Example
    return & config_value_description_list;
    */
    return 0;
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

/*
HydraComponent::~HydraComponent() {

}
*/

// HydraNetComponent
////////////////////

/* Example
const HydraConfigValueDescriptionList HydraNetComponent::config_value_description_list = {
    2, 2 + HYDRA_NET_ROUTE_COUNT * 2, (HydraConfigValueDescription[]) {
        {2, "ADDR"},
        {HYDRA_NET_ROUTE_COUNT * 2, "RouteToNETS"},
    }
};
*/

/* Example
const HydraConfigValueDescriptionList* HydraNetComponent::getConfigDescription() {
    return & config_value_description_list;
}
*/

/* Example
uint8_t* HydraNetComponent::getConfig() {
    return this->config.raw;
}
*/

bool HydraNetComponent::writePacket(const HydraPacket* packet) {
    HydraAddress gateway = this->getGateway(packet->part.to_addr);
    if ((gateway.raw != HYDRA_ADDR_NULL) && (gateway.raw != HYDRA_ADDR_LOCAL)) {
        HydraPacket p;
        memcpy(& p, packet, HYDRA_PACKET_SIZE);
        if (hydra_is_addr_local(packet->part.from_addr)) {
            p.part.from_addr = this->getAddress();
        }
        return this->sendPacket(gateway, & p);
    } else {
        return false;
    }
}

bool HydraNetComponent::sendPacket(const HydraAddress to, const HydraPacket* packet) {
    return false;
}

bool HydraNetComponent::sendPacketFrom(const HydraAddress to, const HydraPacket* packet) {
    HydraPacket p;
    memcpy(& p, packet, HYDRA_PACKET_SIZE);
    p.part.from_addr = this->getAddress();
    return this->sendPacket(to, & p);
}

HydraAddress HydraNetComponent::getGateway(HydraAddress destination) {
    HydraNetConfig* config = (HydraNetConfig*)this->getConfig();
    if (config->parts.addr.part.net == destination.part.net) {
        //to connected network
        if (config->parts.addr.part.device == destination.part.device) {
            //to me
            return (HydraAddress){HYDRA_ADDR_LOCAL};
        } else {
            //can send direct to destination
            return destination;
        }
    } else {
        //other network, check routes
        int i;
        if (destination.part.net != 0) {
            for(i = 0; i < HYDRA_NET_ROUTE_COUNT; ++i) {
                if (config->parts.routes[i].to_net == destination.part.net) {
                    //found route to net via gate in connected network
                    destination.part.net = config->parts.addr.part.net;
                    destination.part.device = config->parts.routes[i].via_device;
                    return destination;
                }
            }
        }
        //no route found
        return (HydraAddress){HYDRA_ADDR_NULL};
    }
}

HydraAddress HydraNetComponent::getAddress() {
    return ((HydraNetConfig*)this->getConfig())->parts.addr;
}

// Hydra
////////

Hydra::Hydra(HydraComponentDescriptionList* components) {
    this->components = components;
    this->ms = (uint32_t) millis();
    this->timestamp = 0;
    this->timezone_offset_seconds = 0;
    this->default_gateway.raw = HYDRA_ADDR_NULL;
    this->led_receive_pin = HYDRA_PIN_NONE;
    this->led_send_pin = HYDRA_PIN_NONE;
}

void Hydra::bootConsole() {
    hydra_fprintln("Send any to enter Hydra configuration console.");
    byte countdown = HYDRA_BOOT_CONSOLE_WAIT_TIME * 10;
    uint8_t led_pin = this->led_receive_pin == HYDRA_PIN_NONE ? 13 : this->led_receive_pin;
    pinMode(led_pin, OUTPUT);
    while (countdown--) {
        delay(100);
        if (Serial.available() > 0) {
            while(Serial.available() > 0) {
                Serial.read();
            }
            hydra_print('\n');
            digitalWrite(led_pin, LOW);
            this->consoleRun();
            return;
        }
        hydra_print('.');
        digitalWrite(led_pin, countdown & 1 ? HIGH : LOW);
    }
    hydra_print('\n');
    digitalWrite(led_pin, LOW);
}

void Hydra::consoleRun() {
    hydra_fprintln("Welcome to Hydra configuration console!");
    this->consoleHelp();
    uint8_t led_pin = this->led_send_pin == HYDRA_PIN_NONE ? 13 : this->led_send_pin;
    pinMode(led_pin, OUTPUT);
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
                    hydra_fprintln("Read from EEPROM");
                    break;
                case 'w':
                    this->saveConfig();
                    hydra_fprintln("Write to EEPROM");
                    break;
                case 's':
                    {
                        int pos = input.indexOf('=');
                        String name = input.substring(2, (unsigned int) pos);
                        name.trim();
                        String value = input.substring((unsigned int) pos + 1);
                        value.trim();
                        if (this->consoleSetConfigValue(name, value)) {
                            hydra_fprintln("Set value successful");
                        }
                    }
                    break;
                case 'd':
                    {
                        int i = 0;
                        while(i < 2048) {
                            this->consolePrintHex((uint8_t)((i >> 8) & 0xff));
                            this->consolePrintHex((uint8_t)(i & 0xff));
                            hydra_print(' ');
                            this->consolePrintHex((uint8_t *) i, 32);
                            hydra_print('\n');
                            i += 32;
                        }
                    }
                    break;
                default:
                    hydra_fprintln("Unknown command");
            }
        }
        digitalWrite(led_pin, millis() & 0x400 ? HIGH : LOW);
    }
    digitalWrite(led_pin, LOW);
}

void Hydra::consoleHelp() {
    hydra_fprintln("Commands:");
    hydra_fprintln("q - Quit");
    hydra_fprintln("h - Help");
    hydra_fprintln("p - Print current values");
    hydra_fprintln("r - Read from EEPROM");
    hydra_fprintln("w - Write to EEPROM");
    hydra_fprintln("s param=hex - Set value, like in 'p'");
}


void Hydra::consolePrintHex(uint8_t value) {
    if (value < 0x10) {
        hydra_print('0');
    }
    hydra_hprint(value);
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
    const char * component_name = item->component->getName();
    if (component_name) {
        name = component_name;
    } else {
        name = "?";
    }
    name.concat("[");
    name.concat(index);
    name.concat("].");
    name.concat(item->component->getConfigDescription()->list[value_index].caption);
    return name;
}

void Hydra::consolePrintConfig() {
    uint8_t i, j;
    for(i = 0; i < this->components->totalCount; ++i) {
        HydraComponentDescription* item = & this->components->list[i];
        const HydraConfigValueDescriptionList* config_description = item->component->getConfigDescription();
        if (config_description) {
            int offset = 0;
            for(j = 0; j < config_description->count; ++j) {
                String name = this->consoleGetConfigValueFullName(i, j);
                hydra_print(name);
                hydra_print('=');
                uint8_t* config = item->component->getConfig();
                this->consolePrintHex(config + offset, config_description->list[j].size);
                hydra_print('\n');
                offset += config_description->list[j].size;
            }
        }
    }
}

bool Hydra::consoleParseHex(String hex, uint8_t* buffer, uint8_t length) {
    if (hex.length() != length * 2) {
        hydra_fprintln("Wrong value length");
        return false;
    }

    uint8_t tmp[length];
    unsigned int i;

    for(i = 0; i < hex.length(); ++i) {
        char c = hex.charAt(i);
        byte b = 0;
        if ((c >= '0') && (c <= '9')) {
            b = (byte)(c - '0');
        } else if ((c >= 'A') && (c <= 'F')) {
            b = (byte)(c - 'A' + 0x0A);
        } else if ((c >= 'a') && (c <= 'f')) {
            b = (byte)(c - 'a' + 0x0A);
        } else {
            hydra_fprint("Wrong value symbol at ");
            hydra_print(i);
            hydra_fprint(" '");
            hydra_print(c);
            hydra_fprintln("'.");
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
    uint8_t i, j;
    for(i = 0; i < this->components->totalCount; ++i) {
        HydraComponentDescription* item = & this->components->list[i];
        const HydraConfigValueDescriptionList* config_description = item->component->getConfigDescription();
        if (config_description) {
            int offset = 0;
            for(j = 0; j < config_description->count; ++j) {
                String pname = this->consoleGetConfigValueFullName(i, j);
                if (pname == name) {
                    return this->consoleParseHex(value, item->component->getConfig() + offset, config_description->list[j].size);
                }
                offset += config_description->list[j].size;
            }
        }
    }
    hydra_fprintln("Param not found");
    return false;
}

void Hydra::loadConfig() {
    hydra_debug("Hydra::loadConfig begin");
    int i, j;
    int offset = 0;
    for(i = 0; i < this->components->totalCount; ++i) {
        HydraComponentDescription* item = & this->components->list[i];
        const HydraConfigValueDescriptionList* config_description = item->component->getConfigDescription();
        if (config_description) {
            uint8_t* config = item->component->getConfig();
            for(j = 0; j < config_description->size; ++j) {
                config[j] = EEPROM.read(offset);
                ++offset;
            }
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
        if (config_description) {
            for(j = 0; j < config_description->size; ++j) {
                uint8_t* config = item->component->getConfig();
                if (config[j] != EEPROM.read(offset)) {
                    EEPROM.write(offset, config[j]);
                }
                ++offset;
            }
        }
    }
    hydra_debug("Hydra::saveConfig end");
}

void Hydra::init(uint8_t random_pin) {
    hydra_debug("Hydra::init begin");
    hydra_fprintln("Hydra v1");
    hydra_fprintln("START");
    this->loadConfig();
    this->bootConsole();

    hydra_fprintln("INIT");
    int i;
    uint8_t service_id = HYDRA_SERVICE_ID_MIN;
    for(i = 0; i < this->components->totalCount; ++i) {
        hydra_debug_param("Hydra::init component ", i);
        if (i >= this->components->netifCount) {
            if (this->components->list[i].id == HYDRA_SERVICE_ID_AUTO) {
                this->components->list[i].id = service_id++;
            }
        }
        this->components->list[i].component->init(this);
    }
    this->master_online_timeout.begin(0);
    this->time_sync_timeout.begin(0);

    // random seed
    this->rand_seed = 0;
    for (byte b = 0; b < 32; ++b) {
        this->rand_seed |= (analogRead(random_pin) & 1) ? 1 : 0;
        this->rand_seed <<= 1;
    }
    hydra_debug_param("random seed ", this->rand_seed);

    hydra_fprintln("READY");
    hydra_debug("Hydra::init end");
}

void Hydra::updateTimer() {
    uint32_t ms = (uint32_t) millis();
    uint32_t delta_ms = ms - this->ms;
    this->timestamp += delta_ms / 1000;
    this->ms = ms - delta_ms % 1000;
}

void Hydra::setPinLedReceive(const uint8_t pin) {
    this->led_receive_pin = pin;
    pinMode(pin, OUTPUT);
}

void Hydra::setPinLedSend(const uint8_t pin) {
    this->led_send_pin = pin;
    pinMode(pin, OUTPUT);
}

void Hydra::onNetReceivePacket() {
    if (this->led_receive_pin != HYDRA_PIN_NONE) {
        digitalWrite(this->led_receive_pin, HIGH);
        this->led_receive_timeout.begin(250);
    }
}

void Hydra::onNetSendPacket() {
    if (this->led_send_pin != HYDRA_PIN_NONE) {
        digitalWrite(this->led_send_pin, HIGH);
        this->led_send_timeout.begin(250);
    }
}

void Hydra::loop() {
    int i;
    HydraPacket packet;

    HydraTimeout::calcDelta();
    this->master_online_timeout.tick();
    this->time_sync_timeout.tick();

    // enumerate services
    for(i = 0; i < this->components->totalCount; ++i) {
        this->updateTimer();

        HydraComponent* item = this->components->list[i].service.component;
        if (item->isPacketAvailable()) {
            memset(& packet, 0, HYDRA_PACKET_SIZE);
            if (item->readPacket(& packet)) {
                HydraAddress received_via;
                if (i < this->components->netifCount) {
                    this->onNetReceivePacket();
                    received_via.raw = ((HydraNetComponent*) item)->getAddress().raw;
                } else {
                    received_via.raw = HYDRA_ADDR_LOCAL;
                    packet.part.from_addr.raw = HYDRA_ADDR_LOCAL;
                    packet.part.from_service = this->components->list[i].id;
                    packet.part.timestamp = this->getTime();
                }
                this->route(& packet, received_via);
            }
        }
    }

    if (this->led_receive_pin != HYDRA_PIN_NONE) {
        this->led_receive_timeout.tick();
        if (this->led_receive_timeout.isEnd()) {
             digitalWrite(this->led_receive_pin, LOW);
        }
    }

    if (this->led_send_pin != HYDRA_PIN_NONE) {
        this->led_send_timeout.tick();
        if (this->led_send_timeout.isEnd()) {
             digitalWrite(this->led_send_pin, LOW);
        }
    }
}

void Hydra::netSend(HydraNetComponent* item, const HydraAddress destination, const HydraPacket* packet, const bool need_set_from_addr) {
    this->onNetSendPacket();
    if (need_set_from_addr) {
         item->sendPacketFrom(destination, packet);
    } else {
         item->sendPacket(destination, packet);
    }
}

void Hydra::route(const HydraPacket* packet, const HydraAddress received_via) {
    hydra_debug("Hydra::route begin");
    hydra_debug_param("Hydra::route packet from_addr ", packet->part.from_addr.raw);
    hydra_debug_param("Hydra::route packet to_addr ", packet->part.to_addr.raw);
    hydra_debug_param("Hydra::route packet from_service ", packet->part.from_service);
    hydra_debug_param("Hydra::route packet to_service ", packet->part.to_service);
    hydra_debug_param("Hydra::route packet timestamp ", packet->part.timestamp);
    #ifdef HYDRA_DEBUG
        hydra_fprint("Packet raw: ");
        this->consolePrintHex((uint8_t*)packet, HYDRA_PACKET_SIZE);
        hydra_print('\n');
    #endif
    int i;

    HydraAddress destination = packet->part.to_addr;
    bool need_set_from_addr = (hydra_is_addr_local(received_via) && (hydra_is_addr_local(packet->part.from_addr) || hydra_is_addr_null(packet->part.from_addr)));
    HydraAddress routed_via = (HydraAddress){HYDRA_ADDR_NULL};

    if (hydra_is_addr_null(destination)) {
        //drop
    } else if (hydra_is_addr_local(destination)) {
        this->landing(packet, received_via, (HydraAddress){HYDRA_ADDR_LOCAL});
        routed_via = (HydraAddress){HYDRA_ADDR_LOCAL};

    } else if (hydra_is_addr_to_all(destination)) {
        //send to all nets (exclude source)
        for(i = 0; i < this->components->netifCount; ++i) {
            HydraNetComponent* item = this->components->list[i].netif.component;
            if (received_via.raw != item->getAddress().raw) {
                this->netSend(item, destination, packet, need_set_from_addr);
                routed_via = destination;
            }
        }
        //and to me
        this->landing(packet, received_via, received_via);
        if (hydra_is_addr_null(routed_via)) {
            routed_via = (HydraAddress){HYDRA_ADDR_LOCAL};
        }

    } else if (hydra_is_addr_to_net(destination)) {
        //send to exactly to net (exclude source)
        for(i = 0; i < this->components->netifCount; ++i) {
            HydraNetComponent* item = this->components->list[i].netif.component;
            HydraAddress gateway = item->getGateway(packet->part.to_addr);
            if (gateway.raw != HYDRA_ADDR_NULL) {
                HydraAddress if_addr = item->getAddress();
                if (received_via.raw != if_addr.raw) {
                    //send other devices or net
                    this->netSend(item, gateway, packet, need_set_from_addr);
                    routed_via = gateway;
                }
                if (hydra_is_addr_to_net(gateway) && (if_addr.part.net == destination.part.net)) {
                    //and to me
                    this->landing(packet, received_via, if_addr);
                    routed_via = (HydraAddress){HYDRA_ADDR_LOCAL};
                }
                break;
            }
        }

    } else if (hydra_is_addr_to_devices(destination)) {
        //send to all nets (exclude source)
        bool landed = false;
        for(i = 0; i < this->components->netifCount; ++i) {
            HydraNetComponent* item = this->components->list[i].netif.component;
            HydraAddress if_addr = item->getAddress();
            if (received_via.raw != if_addr.raw) {
                //to other net
                this->netSend(item, destination, packet, need_set_from_addr);
                routed_via = destination;
            }
            if (!landed && (destination.part.device == if_addr.part.device)) {
                //and me
                landed = true;
                this->landing(packet, received_via, if_addr);
                routed_via = (HydraAddress){HYDRA_ADDR_LOCAL};
            }
        }

    } else {
        for(i = 0; i < this->components->netifCount; ++i) {
            HydraNetComponent* item = this->components->list[i].netif.component;
            HydraAddress gateway = item->getGateway(packet->part.to_addr);
            HydraAddress if_addr = item->getAddress();
            if (gateway.raw == HYDRA_ADDR_LOCAL) {
                this->landing(packet, received_via, if_addr);
                routed_via = (HydraAddress){HYDRA_ADDR_LOCAL};
                break;
            } else if (gateway.raw != HYDRA_ADDR_NULL) {
                if (received_via.raw != if_addr.raw) {
                    this->netSend(item, gateway, packet, need_set_from_addr);
                    routed_via = gateway;
                    break;
                }
            }
        }

    }

    if (packet->part.to_service == HYDRA_ECHO_SERVICE_ID) {
        this->pingDebug(packet, received_via, routed_via);
    }

    hydra_debug("Hydra::route end");
}

void Hydra::pingDebug(const HydraPacket* packet, const HydraAddress received_via, const HydraAddress routed_via) {
    if (packet->part.payload.type == HYDRA_ECHO_PAYLOAD_TYPE_REQUEST) {
        hydra_fprint("PING ");
    } else if (packet->part.payload.type == HYDRA_ECHO_PAYLOAD_TYPE_REPLY) {
        hydra_fprint("PONG ");
    } else {
        hydra_fprint("P?NG ");
    }
    hydra_hprint(packet->part.from_addr.raw);
    hydra_fprint(" via ");
    hydra_hprint(received_via.raw);
    hydra_fprint(" -> ");
    hydra_hprint(packet->part.to_addr.raw);
    hydra_fprint(" via ");
    if (hydra_is_addr_null(routed_via)) {
        hydra_fprintln("NO ROUTE");
    } else {
        hydra_hprintln(routed_via.raw);
    }
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
    return !this->time_sync_timeout.isEnd();
}

void Hydra::setTime(uint32_t timestamp, int16_t timezone_offset_minutes) {
    this->timestamp = timestamp;
    if ((timezone_offset_minutes <= 720) || (timezone_offset_minutes >= -720)) {
        this->timezone_offset_seconds = timezone_offset_minutes;
        this->timezone_offset_seconds *= 60;
    }
}

HydraAddress Hydra::getDefaultGateway() {
    return this->default_gateway;
}

bool Hydra::isMasterOnline() {
    return !this->master_online_timeout.isEnd();
}

uint32_t Hydra::rand() {
    //srandom(this->rand_seed ^ millis());
    //this->rand_seed = (uint32_t) random();
    return ~this->rand_seed;
}

uint16_t HydraTimeout::last_ms = 0;
uint16_t HydraTimeout::delta_ms = 0;

void HydraTimeout::begin(uint16_t ms) {
    this->last_ms = (uint32_t) millis();
    this->left_ms = ms;
}

void HydraTimeout::tick() {
    if (this->left_ms) {
        this->left_ms -= (HydraTimeout::delta_ms <= this->left_ms) ? HydraTimeout::delta_ms : this->left_ms;
    }
}

bool HydraTimeout::isEnd() {
    return !this->left_ms;
}

uint16_t HydraTimeout::left() {
    return this->left_ms;
}

void HydraTimeout::calcDelta() {
    uint16_t time = (uint32_t) millis();
    HydraTimeout::delta_ms = time - HydraTimeout::last_ms;
    HydraTimeout::last_ms = time;
}
