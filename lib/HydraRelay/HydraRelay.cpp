#include "HydraRelay.h"
#include <Arduino.h>

const char* HydraRelay::name = "Relay";

HydraRelay::HydraRelay(uint8_t out_pin) {
	this->reply_ready = false;
	this->reply_to_address.raw = HYDRA_ADDR_NULL;
	this->reply_to_service = 0;
	this->out_pin = out_pin;
	this->state = HYDRA_RELAY_STATE_OFF;
	this->next_state = HYDRA_RELAY_STATE_OFF;
	this->switch_timeout.begin(0);
}

const char* HydraRelay::getName() {
	return HydraRelay::name;
}

void HydraRelay::init(Hydra* hydra) {
	hydra_debug("HydraRelay::init begin");
	HydraComponent::init(hydra);
	pinMode(this->out_pin, OUTPUT);
	digitalWrite(this->out_pin, HIGH);
	this->state = HYDRA_RELAY_STATE_OFF;
	this->reply_ready = false;
	hydra_debug("HydraRelay::init end");
}

bool HydraRelay::writePacket(const HydraPacket* packet) {
	hydra_debug("HydraRelay::writePacket");
	switch(packet->part.payload.type) {
	case HYDRA_RELAY_PAYLOAD_TYPE_REQUEST_STATE:
		this->reply_to_address = packet->part.from_addr;
		this->reply_to_service = packet->part.from_service;
		this->reply_ready = true;
		break;
	case HYDRA_RELAY_PAYLOAD_TYPE_COMMAND:
		this->reply_to_address = packet->part.from_addr;
		this->reply_to_service = packet->part.from_service;
		this->next_state = packet->part.payload.data[0];
		this->reply_ready = true;
		hydra_debug_param("Relay: State received:", this->next_state);
		break;
	}
	return true;
}

bool HydraRelay::isPacketAvailable() {
	this->loop();

	return this->reply_ready;
}

void HydraRelay::loop() {
	this->switch_timeout.tick();
	if (this->state != this->next_state) {
		if (this->switch_timeout.isEnd()) {
			hydra_debug_param("Relay: State changed:", this->next_state);
			if (this->next_state == HYDRA_RELAY_STATE_ON) {
				digitalWrite(this->out_pin, LOW);
			} else {
				digitalWrite(this->out_pin, HIGH);
			}
			this->state = this->next_state;
			this->reply_ready = true;
			this->switch_timeout.begin(HYDRA_RELAY_SWITCH_TIMEOUT);
		}
	}
}

bool HydraRelay::readPacket(HydraPacket* packet) {
	if (this->reply_ready) {
		packet->part.to_addr = this->reply_to_address;
		packet->part.to_service = this->reply_to_service;
		packet->part.payload.type = HYDRA_RELAY_PAYLOAD_TYPE_REPLY_STATE;
		packet->part.payload.data[0] = this->state;
		this->reply_ready = false;
		return true;
	}
	return false;
}
