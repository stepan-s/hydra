#include "HydraLm35.h"

const char* HydraLm35::name = "Lm35";

HydraLm35::HydraLm35(uint8_t pin) {
    this->reply_ready = false;
    this->pin = pin;
}

const char* HydraLm35::getName() {
    return HydraLm35::name;
}

void HydraLm35::init(Hydra* hydra) {
    hydra_debug("HydraLm35::init begin");
    HydraComponent::init(hydra);
    analogReference(INTERNAL);
    hydra_debug("HydraLm35::init end");
}

bool HydraLm35::writePacket(const HydraPacket* packet) {
    hydra_debug("HydraLm35::writePacket");
    if (packet->part.payload.type == HYDRA_LM35_PAYLOAD_TYPE_REQUEST) {
        this->reply_to.addr = packet->part.from_addr;
        this->reply_to.service = packet->part.from_service;
        this->reply_ready = true;
    }
    return true;
}

bool HydraLm35::isPacketAvailable() {
    return this->reply_ready;
}

bool HydraLm35::readPacket(HydraPacket* packet) {
    if (this->reply_ready) {
        int value = analogRead(this->pin);
        float temp_hi;
        uint8_t temp_lo = floor(modff(value / 9.31, &temp_hi) * 100);

        packet->part.payload.data[0] = temp_lo;
        packet->part.payload.data[1] = floor(temp_hi);
        packet->part.payload.data[2] = (byte)(value & 0xFF);
        packet->part.payload.data[3] = (byte)(value << 8);

        packet->part.to_addr = this->reply_to.addr;
        packet->part.to_service = this->reply_to.service;
        packet->part.payload.type = HYDRA_LM35_PAYLOAD_TYPE_REPLY;
        this->reply_ready = false;
        return true;
    } else {
        return false;
    }
}
