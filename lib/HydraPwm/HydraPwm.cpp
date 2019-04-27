#include "HydraPwm.h"

const char* HydraPwm::name = "Pwm";

HydraPwm::HydraPwm(uint8_t out_pin) {
    this->reply_ready = false;
    this->reply_to_address.raw = HYDRA_ADDR_NULL;
    this->reply_to_service = 0;
    this->out_pin = out_pin;
    this->value = 0;
}

const char* HydraPwm::getName() {
    return HydraPwm::name;
}

void HydraPwm::init(Hydra* hydra) {
    hydra_debug("HydraPwm::init begin");
    HydraComponent::init(hydra);
    this->value = 0;
    analogWrite(this->out_pin, this->value);
    this->reply_ready = false;
    hydra_debug("HydraPwm::init end");
}

bool HydraPwm::writePacket(const HydraPacket* packet) {
    hydra_debug("HydraPwm::writePacket");
    switch(packet->part.payload.type) {
    case HYDRA_PWM_PAYLOAD_TYPE_REQUEST_STATE:
        this->reply_to_address = packet->part.from_addr;
        this->reply_to_service = packet->part.from_service;
        this->reply_ready = true;
        break;
    case HYDRA_PWM_PAYLOAD_TYPE_COMMAND:
        this->reply_to_address = packet->part.from_addr;
        this->reply_to_service = packet->part.from_service;
        this->value = packet->part.payload.data[0];
        analogWrite(this->out_pin, this->value);
        this->reply_ready = true;
        hydra_debug_param("Pwm: State received:", this->next_state);
        break;
    }
    return true;
}

bool HydraPwm::isPacketAvailable() {
    return this->reply_ready;
}

bool HydraPwm::readPacket(HydraPacket* packet) {
    if (this->reply_ready) {
        packet->part.to_addr = this->reply_to_address;
        packet->part.to_service = this->reply_to_service;
        packet->part.payload.type = HYDRA_PWM_PAYLOAD_TYPE_REPLY_STATE;
        packet->part.payload.data[0] = this->value;
        this->reply_ready = false;
        return true;
    }
    return false;
}
