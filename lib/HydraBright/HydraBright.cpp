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
        uint8_t diapasone = this->config.parts.gain_max - this->config.parts.gain_min;
        uint8_t val = ((value << 6) / diapasone) - ((this->config.parts.gain_min << 8) / diapasone);
        packet->part.payload.data[0] = val;

        packet->part.to_addr = this->reply_to.addr;
        packet->part.to_service = this->reply_to.service;
        packet->part.payload.type = HYDRA_BRIGHT_PAYLOAD_TYPE_REPLY;
        this->reply_ready = false;
        return true;
    } else {
        return false;
    }
}
