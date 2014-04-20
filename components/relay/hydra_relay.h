#ifndef _HYDRA_RELAY_
#define _HYDRA_RELAY_ true

#define HYDRA_SERVICE_RELAY 60
#define HYDRA_PAYLOAD_RELAY_TYPE_REQUEST_STATE 0
#define HYDRA_PAYLOAD_RELAY_TYPE_REPLY_STATE 1
#define HYDRA_PAYLOAD_RELAY_TYPE_COMMAND 2

#define HYDRA_RELAY_SWITCH_TIMEOUT 2000

#define HYDRA_RELAY_STATE_OFF 0
#define HYDRA_RELAY_STATE_ON 1

#include "hydra.h"

class HydraRelay: public HydraComponent {
	static const char* name;
	bool reply_ready;
	HydraAddress reply_to_address;
	uint8_t reply_to_service;

	uint8_t out_pin;
	uint8_t state;
	uint8_t next_state;
	HydraTimeout switch_timeout;
	void loop();

public:
	HydraRelay(uint8_t out_pin1);
	virtual const char* getName();
	virtual void init(Hydra* hydra);
	virtual bool writePacket(const HydraPacket* packet);
	virtual bool isPacketAvailable();
	virtual bool readPacket(HydraPacket* packet);
};

#endif
