#include "hydra_blinds_dc.h"
#include <Arduino.h>

const char* HydraBlindsDc::name = "Blinds";

const HydraConfigValueDescriptionList HydraBlindsDc::config_value_description_list = {
	1, 1, (HydraConfigValueDescription[]) {
		{HYDRA_CONFIG_VALUE_TYPE_INT, 1, "Threshold"},
	}
};

HydraBlindsDcMotor::HydraBlindsDcMotor(uint8_t sensor_pin, uint8_t motor_pin1, uint8_t motor_pin2, uint8_t sensor_threshold) {
	this->sensor_pin = sensor_pin;
	this->sensor_threshold = sensor_threshold;
	this->motor_pin1 = motor_pin1;
	this->motor_pin2 = motor_pin2;
	this->motor_state = HYDRA_BLINDS_DC_MOTOR_STATE_STOP;
	this->position = 0;
	this->position_dest = 0;
	pinMode(this->motor_pin1, OUTPUT);
	pinMode(this->motor_pin2, OUTPUT);
	//this->stop();
}

void HydraBlindsDcMotor::control_loop() {
	this->position = map(analogRead(this->sensor_pin), 0, 1023, -100, 100);
	uint8_t motor_state_next;
	int delta_pos = this->position_dest - this->position;

	if (delta_pos > this->sensor_threshold) {
		motor_state_next = HYDRA_BLINDS_DC_MOTOR_STATE_ROTATE_CW;
	} else if (delta_pos < -this->sensor_threshold) {
		motor_state_next = HYDRA_BLINDS_DC_MOTOR_STATE_ROTATE_CCW;
	} else {
		motor_state_next = HYDRA_BLINDS_DC_MOTOR_STATE_STOP;
	}

	if (this->motor_state != motor_state_next) {
		switch (motor_state_next) {
		case HYDRA_BLINDS_DC_MOTOR_STATE_STOP:
			digitalWrite(this->motor_pin1, LOW);
			digitalWrite(this->motor_pin2, LOW);

			/*if (!hydra_is_addr_null(this->reply_to_address)) {
				this->reply_ready = true;
			}*/
			break;
		case HYDRA_BLINDS_DC_MOTOR_STATE_ROTATE_CW:
			digitalWrite(this->motor_pin1, HIGH);
			digitalWrite(this->motor_pin2, LOW);
			break;
		case HYDRA_BLINDS_DC_MOTOR_STATE_ROTATE_CCW:
			digitalWrite(this->motor_pin1, LOW);
			digitalWrite(this->motor_pin2, HIGH);
			break;
		}
		this->motor_state = motor_state_next;
	}
}

void HydraBlindsDcMotor::stop() {
	digitalWrite(this->motor_pin1, LOW);
	digitalWrite(this->motor_pin2, LOW);
	this->position_dest = this->position = map(analogRead(this->sensor_pin), 0, 1023, -100, 100);
	this->motor_state = HYDRA_BLINDS_DC_MOTOR_STATE_STOP;
}

void HydraBlindsDcMotor::setDestionationPosition(int8_t position) {
	this->position_dest = position;
}


HydraBlindsDc::HydraBlindsDc(uint8_t sensor1_pin, uint8_t motor1_pin1, uint8_t motor1_pin2, uint8_t sensor2_pin, uint8_t motor2_pin1, uint8_t motor2_pin2) {
	this->reply_ready = false;
	this->reply_to_address = (HydraAddress){HYDRA_ADDR_NULL};
	this->reply_to_service = 0;

	uint8_t threshold = this->config.parts.threshold;

	if (sensor1_pin && motor1_pin1 && motor1_pin2) {
		this->motor1 = new HydraBlindsDcMotor(sensor1_pin, motor1_pin1, motor1_pin2, threshold);
	} else {
		this->motor1 = 0;
	}
	if (sensor2_pin && motor2_pin1 && motor2_pin2) {
		this->motor2 = new HydraBlindsDcMotor(sensor2_pin, motor2_pin1, motor2_pin2, threshold);
	} else {
		this->motor2 = 0;
	}
}

const char* HydraBlindsDc::getName() {
	return HydraBlindsDc::name;
}

const HydraConfigValueDescriptionList* HydraBlindsDc::getConfigDescription() {
	return & config_value_description_list;
}

uint8_t* HydraBlindsDc::getConfig() {
	return this->config.raw;
}

void HydraBlindsDc::init(Hydra* hydra) {
	hydra_debug("HydraBlinds::init begin");
	HydraComponent::init(hydra);
	this->reply_ready = false;
	hydra_debug("HydraBlinds::init end");
}

bool HydraBlindsDc::writePacket(const HydraPacket* packet) {
	hydra_debug("HydraBlinds::writePacket");
	switch(packet->part.payload.type) {
	case HYDRA_BLINDS_DC_PAYLOAD_TYPE_REQUEST_STATE:
		this->reply_to_address = packet->part.from_addr;
		this->reply_to_service = packet->part.from_service;
		this->reply_ready = true;
		break;
	case HYDRA_BLINDS_DC_PAYLOAD_TYPE_COMMAND:
		this->reply_to_address = packet->part.from_addr;
		this->reply_to_service = packet->part.from_service;
		if (this->motor1) {
			this->motor1->setDestionationPosition(min(max((int8_t)packet->part.payload.data[0], -100), 100));
		}
		if (this->motor2) {
			this->motor2->setDestionationPosition(min(max((int8_t)packet->part.payload.data[1], -100), 100));
		}
		this->reply_ready = true;
		break;
	}
	return true;
}

bool HydraBlindsDc::isPacketAvailable() {
	this->loop();

	return this->reply_ready;
}

void HydraBlindsDc::loop() {
	if (this->motor1) {
		this->motor1->control_loop();
	}
	if (this->motor2) {
		this->motor2->control_loop();
	}
}

bool HydraBlindsDc::readPacket(HydraPacket* packet) {
	if (this->reply_ready) {
		packet->part.to_addr = this->reply_to_address;
		packet->part.to_service = this->reply_to_service;
		packet->part.payload.type = HYDRA_BLINDS_DC_PAYLOAD_TYPE_REPLY_STATE;
		if (this->motor1) {
			packet->part.payload.data[0] = (uint8_t) this->motor1->position;
			packet->part.payload.data[1] = (uint8_t) this->motor1->position_dest;
			packet->part.payload.data[2] = (uint8_t) this->motor1->motor_state;
		} else {
			packet->part.payload.data[0] = 0;
			packet->part.payload.data[1] = 0;
			packet->part.payload.data[2] = HYDRA_BLINDS_DC_MOTOR_STATE_DISABLED;
		}
		if (this->motor2) {
			packet->part.payload.data[3] = (uint8_t) this->motor2->position;
			packet->part.payload.data[4] = (uint8_t) this->motor2->position_dest;
			packet->part.payload.data[5] = (uint8_t) this->motor2->motor_state;
		} else {
			packet->part.payload.data[3] = 0;
			packet->part.payload.data[4] = 0;
			packet->part.payload.data[5] = HYDRA_BLINDS_DC_MOTOR_STATE_DISABLED;
		}
		this->reply_ready = false;
		return true;
	}
	return false;
}
