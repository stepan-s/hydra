#ifndef _HYDRA_BLINDS_DC_
#define _HYDRA_BLINDS_DC_ true

#define HYDRA_SERVICE_BLINDS_DC 4
#define HYDRA_PAYLOAD_BLINDS_DC_TYPE_REQUEST_STATE 0
#define HYDRA_PAYLOAD_BLINDS_DC_TYPE_REPLY_STATE 1
#define HYDRA_PAYLOAD_BLINDS_DC_TYPE_COMMAND 2

#define HYDRA_BLINDS_DC_MOTOR_STATE_STOP 0
#define HYDRA_BLINDS_DC_MOTOR_STATE_ROTATE_CW 1
#define HYDRA_BLINDS_DC_MOTOR_STATE_ROTATE_CCW 2
#define HYDRA_BLINDS_DC_MOTOR_STATE_DISABLED 3

#include "hydra.h"

struct HydraBlindsDcConfig {
	union {
		uint8_t raw[3];
		struct {
			uint8_t threshold;
		} parts;
	};
};

class HydraBlindsDcMotor {
	uint8_t sensor_pin;
	uint8_t sensor_threshold;
	uint8_t motor_pin1;
	uint8_t motor_pin2;

public:
	uint8_t motor_state;
	int8_t position;
	int8_t position_dest;

	HydraBlindsDcMotor(uint8_t sensor_pin, uint8_t motor_pin1, uint8_t motor_pin2, uint8_t threshold);
	void control_loop();
	void stop();
	void setDestionationPosition(int8_t position);
};

class HydraBlindsDc: public HydraComponent {
	static const char* name;
	static const HydraConfigValueDescriptionList config_value_description_list;
	HydraBlindsDcConfig config;
	bool reply_ready;
	HydraAddress reply_to_address;
	uint8_t reply_to_service;

	HydraBlindsDcMotor* motor1;
	HydraBlindsDcMotor* motor2;
	void loop();

public:
	HydraBlindsDc(uint8_t sensor1_pin, uint8_t motor1_pin1, uint8_t motor1_pin2, uint8_t sensor2_pin, uint8_t motor2_pin1, uint8_t motor2_pin2);
	virtual const char* getName();
	virtual const HydraConfigValueDescriptionList* getConfigDescription();
	virtual uint8_t* getConfig();
	virtual void init(Hydra* hydra);
	virtual bool writePacket(const HydraPacket* packet);
	virtual bool isPacketAvailable();
	virtual bool readPacket(HydraPacket* packet);
};

#endif
