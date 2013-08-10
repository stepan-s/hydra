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
	hydra_debug("HydraEcho::init begin");
	HydraComponent::init(hydra);
	hydra_debug("HydraEcho::init end");
}

bool HydraEcho::writePacket(const HydraPacket* packet) {
	hydra_debug("HydraEcho::writePacket");
	if (packet->part.payload.type == HYDRA_PAYLOAD_ECHO_TYPE_REQUEST) {
		memcpy(& this->reply, packet, HYDRA_PACKET_SIZE);
		//swap to and from
		HydraAddress tmp_addr = this->reply.part.from_addr;
		this->reply.part.from_addr = this->reply.part.to_addr;
		this->reply.part.to_addr = tmp_addr;
		this->reply_ready = true;
		uint16_t tmp_service = this->reply.part.from_service;
		this->reply.part.from_service = this->reply.part.to_service;
		this->reply.part.to_service = tmp_service;
		//reply
		this->reply.part.payload.type = HYDRA_PAYLOAD_ECHO_TYPE_REPLY;
	}
	return true;
}

bool HydraEcho::isPacketAvailable() {
	return this->reply_ready;
}

bool HydraEcho::readPacket(HydraPacket* packet) {
	memcpy(packet, & this->reply, HYDRA_PACKET_SIZE);
	this->reply_ready = false;
	return true;
}
