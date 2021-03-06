#include "HydraBright.h"

const char* HydraBright::name = "Bright";

HydraBright::HydraBright(uint8_t pin) {
    this->reply_ready = false;
    this->pin = pin;
}

const HydraConfigValueDescriptionList HydraBright::config_value_description_list = {
    2, 2, (HydraConfigValueDescription[]) {
        {HYDRA_CONFIG_VALUE_TYPE_INT, 1, "GainMin"},
        {HYDRA_CONFIG_VALUE_TYPE_INT, 1, "GainMax"},
    }
};

const char* HydraBright::getName() {
    return HydraBright::name;
}

const HydraConfigValueDescriptionList* HydraBright::getConfigDescription() {
    return & config_value_description_list;
}

uint8_t* HydraBright::getConfig() {
    return this->config.raw;
}

bool HydraBright::writePacket(const HydraPacket* packet) {
    hydra_debug("HydraBright::writePacket");
    if (packet->part.payload.type == HYDRA_BRIGHT_PAYLOAD_TYPE_REQUEST) {
        this->reply_to.addr = packet->part.from_addr;
        this->reply_to.service = packet->part.from_service;
        this->reply_ready = true;
    }
    return true;
}

bool HydraBright::isPacketAvailable() {
    return this->reply_ready;
}

bool HydraBright::readPacket(HydraPacket* packet) {
    if (this->reply_ready) {
        int value = analogRead(this->pin);
        int value_gain_min = this->config.parts.gain_min << 2;
        int value_gain_max = this->config.parts.gain_max << 2;
        value = max(value, value_gain_min);
        value = min(value, value_gain_max);
        value -= value_gain_min;
        int interval = value_gain_max - value_gain_min;
        if (interval > 0) {
            float multiplier = (float) 256 / (float) interval;
            value *= multiplier;
        } else {
            value = 0;
        }
        packet->part.payload.data[0] = (byte)value;

        packet->part.to_addr = this->reply_to.addr;
        packet->part.to_service = this->reply_to.service;
        packet->part.payload.type = HYDRA_BRIGHT_PAYLOAD_TYPE_REPLY;
        this->reply_ready = false;
        return true;
    } else {
        return false;
    }
}
