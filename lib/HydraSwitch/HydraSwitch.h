#ifndef _HYDRA_SWITCH_
#define _HYDRA_SWITCH_ true

#define HYDRA_SWITCH_PAYLOAD(idx)					 HYDRA_PAYLOAD_TYPE_MIN + 0 + idx
#define HYDRA_SWITCH_PAYLOAD_TYPE_REQUEST_STATE		HYDRA_SWITCH_PAYLOAD(0)
#define HYDRA_SWITCH_PAYLOAD_TYPE_REPLY_STATE		HYDRA_SWITCH_PAYLOAD(1)

#define HYDRA_SWITCH_TIMEOUT 500

#define HYDRA_SWITCH_STATE_OFF 0
#define HYDRA_SWITCH_STATE_ON 1

#include "Hydra.h"

struct HydraSwitchConfig {
	union {
		uint8_t raw[6];
		struct {
			HydraAddressPort master_service;
			HydraAddressPort relay_service;
		} parts;
	};
};

class HydraSwitch: public HydraComponent {
	static const char* name;
	static const HydraConfigValueDescriptionList config_value_description_list;
	HydraSwitchConfig config;
	bool reply_ready;
	bool state_ready;
	HydraAddress reply_to_address;
	uint8_t reply_to_service;

	uint8_t in_pin;
	uint8_t state;
	HydraTimeout switch_timeout;
	void loop();

public:
	HydraSwitch(uint8_t in_pin);
	virtual const char* getName();
	virtual const HydraConfigValueDescriptionList* getConfigDescription();
	virtual uint8_t* getConfig();
	virtual void init(Hydra* hydra);
	virtual bool writePacket(const HydraPacket* packet);
	virtual bool isPacketAvailable();
	virtual bool readPacket(HydraPacket* packet);
};

#endif
