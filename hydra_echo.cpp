#include "hydra_echo.h"
#include <Arduino.h>

const char* HydraEcho::name = "Echo";

const HydraConfigValueDescriptionList HydraEcho::config_value_description_list = {
	0, 0, 0
};

const char* HydraEcho::getName() {
	return HydraEcho::name;
}

const HydraConfigValueDescriptionList* HydraEcho::getConfigDescription() {
	return & config_value_description_list;
}

uint8_t* HydraEcho::getConfig() {
	return 0;
}

void HydraEcho::init(Hydra* hydra) {
	this->reply_ready = false;
	hydra_debug("HydraEcho::init begin");
	HydraComponent::init(hydra);
	hydra_debug("HydraEcho::init end");
}

bool HydraEcho::writePacket(const HydraPacket* packet) {
	hydra_debug("HydraEcho::writePacket");
	if (packet->part.payload.type == HYDRA_PAYLOAD_ECHO_TYPE_REQUEST) {
		memcpy(& this->reply_payload, packet->part.payload.data, HYDRA_PACKET_PAYLOAD_DATA_SIZE);
		this->reply_to_address = packet->part.from_addr;
		this->reply_to_service = packet->part.from_service;
		this->reply_ready = true;
	}
	return true;
}

bool HydraEcho::isPacketAvailable() {
	return this->reply_ready;
}

bool HydraEcho::readPacket(HydraPacket* packet) {
	if (this->reply_ready) {
		memcpy(packet->part.payload.data, & this->reply_payload, HYDRA_PACKET_PAYLOAD_DATA_SIZE);
		packet->part.to_addr = this->reply_to_address;
		packet->part.to_service = this->reply_to_service;
		packet->part.payload.type = HYDRA_PAYLOAD_ECHO_TYPE_REPLY;
		this->reply_ready = false;
		return true;
	} else {
		return false;
	}
}
