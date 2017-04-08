#include "HydraBlinds.h"
#include <Arduino.h>

const char* HydraBlinds::name = "Blinds";

const HydraConfigValueDescriptionList HydraBlinds::config_value_description_list = {
	2, 3, (HydraConfigValueDescription[]) {
		{HYDRA_CONFIG_VALUE_TYPE_INT, 2, "Speed"},
		{HYDRA_CONFIG_VALUE_TYPE_INT, 1, "Threshold"},
	}
};

HydraBlinds::HydraBlinds(uint8_t sensorPin, uint8_t motorPin1, uint8_t motorPin2, uint8_t motorPin3, uint8_t motorPin4) {
	this->reply_ready = false;
	this->reply_to_address = (HydraAddress){HYDRA_ADDR_NULL};
	this->reply_to_service = 0;
	this->sensor_pin = sensorPin;
	this->motor = new AccelStepper(AccelStepper::HALF4WIRE, motorPin1, motorPin2, motorPin3, motorPin4, false);
	this->motor_state = HYDRA_BLINDS_MOTOR_STATE_STOP;
	this->position = 0;
	this->position_dest = 0;
}

const char* HydraBlinds::getName() {
	return HydraBlinds::name;
}

const HydraConfigValueDescriptionList* HydraBlinds::getConfigDescription() {
	return & config_value_description_list;
}

uint8_t* HydraBlinds::getConfig() {
	return this->config.raw;
}

void HydraBlinds::init(Hydra* hydra) {
	hydra_debug("HydraBlinds::init begin");
	HydraComponent::init(hydra);
	this->motor->setMaxSpeed(this->config.parts.speed);
	this->motor->setAcceleration(0.5);
	this->reply_ready = false;
	hydra_debug("HydraBlinds::init end");
}

bool HydraBlinds::writePacket(const HydraPacket* packet) {
	hydra_debug("HydraBlinds::writePacket");
	switch(packet->part.payload.type) {
	case HYDRA_BLINDS_PAYLOAD_TYPE_REQUEST_STATE:
		this->reply_to_address = packet->part.from_addr;
		this->reply_to_service = packet->part.from_service;
		this->reply_ready = true;
		break;
	case HYDRA_BLINDS_PAYLOAD_TYPE_COMMAND:
		this->reply_to_address = packet->part.from_addr;
		this->reply_to_service = packet->part.from_service;
		this->position_dest = min(max((int8_t)packet->part.payload.data[0], -100), 100);
		this->reply_ready = true;
		break;
	}
	return true;
}

bool HydraBlinds::isPacketAvailable() {
	this->loop();

	return this->reply_ready;
}

void HydraBlinds::loop() {
	this->position = map(analogRead(this->sensor_pin), 0, 1023, -100, 100);

	uint8_t motor_state_next;
	int delta_pos = this->position_dest - this->position;
	int threshold = this->config.parts.threshold;
	if (delta_pos > threshold) {
		motor_state_next = HYDRA_BLINDS_MOTOR_STATE_ROTATE_CW;
	} else if (delta_pos < -threshold) {
		motor_state_next = HYDRA_BLINDS_MOTOR_STATE_ROTATE_CCW;
	} else {
		motor_state_next = HYDRA_BLINDS_MOTOR_STATE_STOP;
	}

	if (this->motor_state != motor_state_next) {
		float speed = this->config.parts.speed;
		switch (motor_state_next) {
		case HYDRA_BLINDS_MOTOR_STATE_STOP:
			this->motor->setSpeed(0);
			this->motor->disableOutputs();

			/*if (!hydra_is_addr_null(this->reply_to_address)) {
				this->reply_ready = true;
			}*/
			break;
		case HYDRA_BLINDS_MOTOR_STATE_ROTATE_CW:
			this->motor->setSpeed(speed);
			this->motor->enableOutputs();
			break;
		case HYDRA_BLINDS_MOTOR_STATE_ROTATE_CCW:
			this->motor->setSpeed(-speed);
			this->motor->enableOutputs();
			break;
		}
		this->motor_state = motor_state_next;
	}

	switch (this->motor_state) {
	case HYDRA_BLINDS_MOTOR_STATE_ROTATE_CW:
	case HYDRA_BLINDS_MOTOR_STATE_ROTATE_CCW:
		this->motor->runSpeed();
		break;
	}
}

bool HydraBlinds::readPacket(HydraPacket* packet) {
	if (this->reply_ready) {
		packet->part.to_addr = this->reply_to_address;
		packet->part.to_service = this->reply_to_service;
		packet->part.payload.type = HYDRA_BLINDS_PAYLOAD_TYPE_REPLY_STATE;
		packet->part.payload.data[0] = (uint8_t) this->position;
		packet->part.payload.data[1] = (uint8_t) this->position_dest;
		packet->part.payload.data[2] = (uint8_t) this->motor_state;
		this->reply_ready = false;
		return true;
	}
	return false;
}
