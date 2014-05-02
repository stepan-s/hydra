#include "hydra_echo.h"
#include <Arduino.h>

const char* HydraEcho::name = "Echo";

const HydraConfigValueDescriptionList HydraEcho::config_value_description_list = {
	1, 2, (HydraConfigValueDescription[]) {
		{2, "PingADDR"},
	}
};

const char* HydraEcho::getName() {
	return HydraEcho::name;
}

const HydraConfigValueDescriptionList* HydraEcho::getConfigDescription() {
	return & config_value_description_list;
}

uint8_t* HydraEcho::getConfig() {
	return this->config.raw;
}

HydraEcho::HydraEcho() {
	this->lost = 0;
	this->graph = 0;
}

void HydraEcho::init(Hydra* hydra) {
	hydra_debug("HydraEcho::init begin");
	HydraComponent::init(hydra);
	this->reply_ready = false;
	this->ping_timeout.begin(0);
	hydra_debug("HydraEcho::init end");
}

bool HydraEcho::writePacket(const HydraPacket* packet) {
	hydra_debug("HydraEcho::writePacket");
	switch(packet->part.payload.type) {
	case HYDRA_ECHO_PAYLOAD_TYPE_REQUEST:
		memcpy(& this->reply_payload, packet->part.payload.data, HYDRA_PACKET_PAYLOAD_DATA_SIZE);
		this->reply_to_address = packet->part.from_addr;
		this->reply_to_service = packet->part.from_service;
		this->reply_ready = true;
		hydra_fprint("Ping rx ");
		hydra_hprintln(packet->part.from_addr.raw);
		break;
	case HYDRA_ECHO_PAYLOAD_TYPE_REPLY:
		if ((this->config.parts.addr.raw != HYDRA_ADDR_NULL) && (this->config.parts.addr.raw == packet->part.from_addr.raw)) {
			--this->lost;
			this->graph |= 1;
			hydra_fprint("Pong rx ");
			hydra_hprintln(packet->part.from_addr.raw);
		}
		break;
	}
	return true;
}

bool HydraEcho::isPacketAvailable() {
	this->ping_timeout.tick();
	return this->reply_ready || ((this->config.parts.addr.raw != HYDRA_ADDR_NULL) && (this->ping_timeout.isEnd()));
}

bool HydraEcho::readPacket(HydraPacket* packet) {
	if (this->reply_ready) {
		memcpy(packet->part.payload.data, & this->reply_payload, HYDRA_PACKET_PAYLOAD_DATA_SIZE);
		packet->part.to_addr = this->reply_to_address;
		packet->part.to_service = this->reply_to_service;
		packet->part.payload.type = HYDRA_ECHO_PAYLOAD_TYPE_REPLY;
		this->reply_ready = false;
		hydra_fprint("Pong tx ");
		hydra_hprintln(packet->part.to_addr.raw);
		return true;
	} else if ((this->config.parts.addr.raw != HYDRA_ADDR_NULL) && (this->ping_timeout.isEnd())) {
		hydra_fprint("Lost packets ");
		hydra_print(this->lost);
		hydra_fprint(" [");
		uint64_t graph = this->graph;
		for(uint8_t i = 0; i < 64; ++i) {
			hydra_print((graph & 0x8000000000000000) ? '!' : '.');
			graph <<= 1;
		}
		hydra_fprintln("]");

		++this->lost;
		this->graph <<= 1;

		packet->part.to_addr = this->config.parts.addr;
		packet->part.to_service = HYDRA_SERVICE_ECHO;
		packet->part.payload.type = HYDRA_ECHO_PAYLOAD_TYPE_REQUEST;
		this->ping_timeout.begin(HYDRA_ECHO_PING_TIMEOUT);
		hydra_fprint("Ping tx ");
		hydra_hprintln(packet->part.to_addr.raw);
		return true;
	} else {
		return false;
	}
}
