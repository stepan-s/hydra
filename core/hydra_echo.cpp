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
		Serial.println("Ping rx");
		break;
	case HYDRA_ECHO_PAYLOAD_TYPE_REPLY:
		if ((this->config.parts.addr.raw != HYDRA_ADDR_NULL) && (this->config.parts.addr.raw == packet->part.from_addr.raw)) {
			Serial.println("Pong rx");
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
		Serial.println("Pong tx");
		return true;
	} else if ((this->config.parts.addr.raw != HYDRA_ADDR_NULL) && (this->ping_timeout.isEnd())) {
		packet->part.to_addr = this->config.parts.addr;
		packet->part.to_service = HYDRA_SERVICE_ECHO;
		packet->part.payload.type = HYDRA_ECHO_PAYLOAD_TYPE_REQUEST;
		this->ping_timeout.begin(HYDRA_ECHO_PING_TIMEOUT);
		Serial.println("Ping tx");
		return true;
	} else {
		return false;
	}
}
