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
	this->timestamp = hydra->getTime();
	hydra_debug("HydraEcho::init end");
}

bool HydraEcho::writePacket(const HydraPacket* packet) {
	hydra_debug("HydraEcho::writePacket");
	if (packet->part.payload.type == HYDRA_PAYLOAD_ECHO_TYPE_REQUEST) {
		memcpy(& this->reply_payload, packet->part.payload.data, HYDRA_PACKET_PAYLOAD_DATA_SIZE);
		this->reply_from_address = packet->part.to_addr;
		this->reply_to_address = packet->part.from_addr;
		this->reply_to_service = packet->part.from_service;
		this->reply_ready = true;
	}
	return true;
}

bool HydraEcho::isPacketAvailable() {
	return this->reply_ready || ((this->config.parts.addr.raw != HYDRA_ADDR_NULL) && (this->timestamp < this->hydra->getTime()));
}

bool HydraEcho::readPacket(HydraPacket* packet) {
	if (this->reply_ready) {
		memcpy(packet->part.payload.data, & this->reply_payload, HYDRA_PACKET_PAYLOAD_DATA_SIZE);
		packet->part.from_addr = this->reply_from_address;
		packet->part.to_addr = this->reply_to_address;
		packet->part.to_service = this->reply_to_service;
		packet->part.payload.type = HYDRA_PAYLOAD_ECHO_TYPE_REPLY;
		this->reply_ready = false;
		return true;
	} else if ((this->config.parts.addr.raw != HYDRA_ADDR_NULL) && (this->timestamp < this->hydra->getTime())) {
		packet->part.to_addr = this->config.parts.addr;
		packet->part.to_service = HYDRA_SERVICE_ECHO;
		packet->part.payload.type = HYDRA_PAYLOAD_ECHO_TYPE_REQUEST;
		this->timestamp = hydra->getTime();
		return true;
	} else {
		return false;
	}
}
