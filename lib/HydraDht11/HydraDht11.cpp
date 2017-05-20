#include "HydraDht11.h"

const char* HydraDht11::name = "Dht11";

HydraDht11::HydraDht11(uint8_t pin) {
    this->reply_ready = false;
    this->pin = pin;
}

const char* HydraDht11::getName() {
    return HydraDht11::name;
}

void HydraDht11::init(Hydra* hydra) {
    hydra_debug("HydraDht11::init begin");
    HydraComponent::init(hydra);
    hydra_debug("HydraDht11::init end");
}

bool HydraDht11::writePacket(const HydraPacket* packet) {
    hydra_debug("HydraDht11::writePacket");
    if (packet->part.payload.type == HYDRA_DHT11_PAYLOAD_TYPE_REQUEST) {
        this->reply_to_address = packet->part.from_addr;
        this->reply_to_service = packet->part.from_service;
        this->reply_ready = true;
    }
    return true;
}

bool HydraDht11::isPacketAvailable() {
    return this->reply_ready;
}

bool HydraDht11::readPacket(HydraPacket* packet) {
    if (this->reply_ready) {
        int chk = this->sensor.read(this->pin);
        switch (chk) {
          case DHTLIB_OK:
              memcpy(packet->part.payload.data, & this->sensor.temperature, sizeof(this->sensor.temperature));
              memcpy(packet->part.payload.data + sizeof(this->sensor.temperature), & this->sensor.humidity, sizeof(this->sensor.humidity));
              break;
          case DHTLIB_ERROR_CHECKSUM:
          case DHTLIB_ERROR_TIMEOUT:
          default:
              return false;
        }

        packet->part.to_addr = this->reply_to_address;
        packet->part.to_service = this->reply_to_service;
        packet->part.payload.type = HYDRA_DHT11_PAYLOAD_TYPE_REPLY;
        this->reply_ready = false;
        return true;
    } else {
        return false;
    }
}
