#include "hydra_config.h"
#include <Arduino.h>

const char* HydraConfig::name = "Conf";

const char* HydraConfig::getName() {
	return HydraConfig::name;
}

void HydraConfig::init(Hydra* hydra) {
	hydra_debug("HydraConfig::init begin");
	HydraComponent::init(hydra);
	this->reply_type = 0;
	hydra_debug("HydraConfig::init end");
}

bool HydraConfig::writePacket(const HydraPacket* packet) {
	hydra_debug("HydraConfig::writePacket");
	this->reply_type = packet->part.payload.type + 1;
	this->service_index = packet->part.payload.data[0];
	this->value_index = packet->part.payload.data[1];
	switch (packet->part.payload.type) {
	case HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_COUNT:
	case HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_ID:
	case HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_NAME:
	case HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_CONFIG_COUNT:
	case HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_CONFIG_NAME:
	case HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_CONFIG_GET_VALUE:
		break;
	case HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_CONFIG_SET_VALUE:
		uint8_t* data;
		uint8_t size;
		if (this->getConfigValuePointer(this->service_index, this->value_index, & data, & size)) {
			memcpy(
				data,
				& packet->part.payload.data[3],
				min(size, HYDRA_PACKET_PAYLOAD_DATA_SIZE - 4)
			);
		}
		break;
	case HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_CONFIG_READ:
		this->hydra->loadConfig();
		break;
	case HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_CONFIG_WRITE:
		this->hydra->saveConfig();
		break;
	default:
		this->reply_type = 0;
		return false;
	}
	return true;
}

bool HydraConfig::isPacketAvailable() {
	return this->reply_type;
}

bool HydraConfig::readPacket(HydraPacket* packet) {
	HydraComponentDescriptionList* components = this->hydra->components;
	bool result = false;
	switch(this->reply_type) {
	case HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_COUNT:
		packet->part.payload.data[0] = components->netifCount;
		packet->part.payload.data[1] = components->serviceCount;
		break;
	case HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_ID:
		if (this->service_index < components->totalCount) {
			packet->part.payload.data[0] = this->service_index;
			packet->part.payload.data[1] = components->list[this->service_index].service.id;
			result = true;
		}
		break;
	case HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_NAME:
		if (this->service_index < components->totalCount) {
			packet->part.payload.data[0] = this->service_index;
			const char *name = components->list[this->service_index].service.component->getName();
			if (name) {
				memcpy(& packet->part.payload.data[1], name, min(strlen(name), HYDRA_PACKET_PAYLOAD_DATA_SIZE - 2));
				packet->part.payload.data[HYDRA_PACKET_PAYLOAD_DATA_SIZE - 1] = 0;
			} else {
				packet->part.payload.data[1] = 0;
			}
			result = true;
		}
		break;
	case HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_CONFIG_COUNT:
		if (this->service_index < components->totalCount) {
			packet->part.payload.data[0] = this->service_index;
			const HydraConfigValueDescriptionList *config_value_description_list = components->list[this->service_index].service.component->getConfigDescription();
			packet->part.payload.data[1] = config_value_description_list ? config_value_description_list->count : 0;
			result = true;
		}
		break;
	case HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_CONFIG_NAME:
		if (this->service_index < components->totalCount) {
			const HydraConfigValueDescriptionList *config_value_description_list = components->list[this->service_index].service.component->getConfigDescription();
			if (config_value_description_list && (this->value_index < config_value_description_list->count)) {
				packet->part.payload.data[0] = this->service_index;
				packet->part.payload.data[1] = this->value_index;
				const char *caption = config_value_description_list->list[this->value_index].caption;
				memcpy(& packet->part.payload.data[2], caption, min(strlen(caption), HYDRA_PACKET_PAYLOAD_DATA_SIZE - 3));
				packet->part.payload.data[HYDRA_PACKET_PAYLOAD_DATA_SIZE - 1] = 0;
				result = true;
			}
		}
		break;
	case HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_CONFIG_GET_VALUE:
	case HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_CONFIG_SET_VALUE:
		uint8_t* data;
		uint8_t size;
		if (this->getConfigValuePointer(this->service_index, this->value_index, & data, & size)) {
			packet->part.payload.data[0] = this->service_index;
			packet->part.payload.data[1] = this->value_index;
			packet->part.payload.data[2] = size;
			memcpy(
				& packet->part.payload.data[3],
				data,
				min(size, HYDRA_PACKET_PAYLOAD_DATA_SIZE - 3)
			);
			result = true;
		}
		break;
	case HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_CONFIG_READ:
		result = true;
		break;
	case HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_CONFIG_WRITE:
		result = true;
		break;
	default:
		return false;
	}
	packet->part.payload.type = this->reply_type;
	return result;
}

bool HydraConfig::getConfigValuePointer(const uint8_t service_index, const uint8_t value_index, uint8_t** out_pointer, uint8_t* out_size) {
	HydraComponentDescriptionList* components = this->hydra->components;
	if (service_index < components->totalCount) {
		const HydraConfigValueDescriptionList *config_value_description_list = components->list[service_index].service.component->getConfigDescription();
		if (config_value_description_list && (value_index < config_value_description_list->count)) {
			int offset = 0;
			for(uint8_t j = 0; j < value_index; ++j) {
				offset += config_value_description_list->list[j].size;
			}
			*out_pointer = components->list[this->service_index].service.component->getConfig() + offset;
			*out_size = config_value_description_list->list[value_index].size;
			return true;
		}
	}
	return false;
}
