#include "hydra_core.h"
#include <Arduino.h>

const char* HydraCore::name = "Core";

const HydraConfigValueDescriptionList HydraCore::config_value_description_list = {
	0, 0, 0
};

const char* HydraCore::getName() {
	return HydraCore::name;
}

const HydraConfigValueDescriptionList* HydraCore::getConfigDescription() {
	return & config_value_description_list;
}

uint8_t* HydraCore::getConfig() {
	return 0;
}

void HydraCore::init(Hydra* hydra) {
	hydra_debug("HydraCore::init begin");
	HydraComponent::init(hydra);
	hydra_debug("HydraCore::init end");
}

bool HydraCore::writePacket(const HydraPacket* packet) {
	hydra_debug("HydraCore::writePacket");
	switch (packet->part.payload.type) {
	case HYDRA_PAYLOAD_CORE_TYPE_SET_TIME:
		hydra_debug_param("HydraCore::writePacket setTime: ", packet->part.timestamp);
		this->hydra->setTime(packet->part.timestamp);
		break;
	default:
		return false;
	}
	return true;
}

bool HydraCore::isPacketAvailable() {
	return false;
}

bool HydraCore::readPacket(HydraPacket* packet) {
	return false;
}
