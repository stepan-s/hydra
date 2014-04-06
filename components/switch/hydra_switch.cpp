#include "hydra_switch.h"
#include "hydra_relay.h"
#include <Arduino.h>

const char* HydraSwitch::name = "Switch";

const HydraConfigValueDescriptionList HydraSwitch::config_value_description_list = {
	2, 6, (HydraConfigValueDescription[]) {
		{3, "ControllerADDRPORT"},
		{3, "DirectADDRPORT"},
	}
};


HydraSwitch::HydraSwitch(uint8_t in_pin) {
	this->reply_ready = false;
	this->reply_to_address = {HYDRA_ADDR_NULL};
	this->reply_to_service = 0;
	this->in_pin = in_pin;
	this->switch_timeout = 0;
}

const char* HydraSwitch::getName() {
	return HydraSwitch::name;
}

const HydraConfigValueDescriptionList* HydraSwitch::getConfigDescription() {
	return & config_value_description_list;
}

uint8_t* HydraSwitch::getConfig() {
	return this->config.raw;
}

void HydraSwitch::init(Hydra* hydra) {
	hydra_debug("HydraSwitch::init begin");
	HydraComponent::init(hydra);
	pinMode(this->in_pin, INPUT);
	this->state = digitalRead(this->in_pin) ? HYDRA_SWITCH_STATE_ON : HYDRA_SWITCH_STATE_OFF;
	this->reply_ready = false;
	hydra_debug("HydraSwitch::init end");
}

bool HydraSwitch::writePacket(const HydraPacket* packet) {
	hydra_debug("HydraSwitch::writePacket");
	switch(packet->part.payload.type) {
	case HYDRA_PAYLOAD_SWITCH_TYPE_REQUEST_STATE:
		this->reply_to_address = packet->part.from_addr;
		this->reply_to_service = packet->part.from_service;
		this->reply_ready = true;
		break;
	}
	return true;
}

bool HydraSwitch::isPacketAvailable() {
	this->loop();

	return this->reply_ready;
}

void HydraSwitch::loop() {
	uint8_t state = digitalRead(this->in_pin) ? HYDRA_SWITCH_STATE_ON : HYDRA_SWITCH_STATE_OFF;
	if (state != this->state) {
		//FIXME: overflow
		if (millis() >= this->switch_timeout) {
			this->state = state;
			this->state_ready;
			this->switch_timeout = millis() + HYDRA_SWITCH_TIMEOUT;
		}
	}
}

bool HydraSwitch::readPacket(HydraPacket* packet) {
	if (this->reply_ready) {
		packet->part.to_addr = this->reply_to_address;
		packet->part.to_service = this->reply_to_service;
		packet->part.payload.type = HYDRA_PAYLOAD_SWITCH_TYPE_REPLY_STATE;
		packet->data[0] = this->state;
		this->reply_ready = false;
		return true;
	}
	if (this->state_ready) {
		if (this->hydra->isMasterOnline()) {
			packet->part.to_addr = this->config.parts.master_service.addr;
			packet->part.to_service = this->config.parts.master_service.service;
			packet->part.payload.type = HYDRA_PAYLOAD_SWITCH_TYPE_REPLY_STATE;
			packet->data[0] = this->state;
		} else {
			packet->part.to_addr = this->config.parts.relay_service.addr;
			packet->part.to_service = this->config.parts.relay_service.service;
			packet->part.payload.type = HYDRA_PAYLOAD_RELAY_TYPE_COMMAND;
			packet->data[0] = (this->state == HYDRA_SWITCH_STATE_ON) ? HYDRA_RELAY_STATE_ON : HYDRA_RELAY_STATE_OFF;
		}
		this->state_ready = false;
		return true;
	}
	return false;
}
