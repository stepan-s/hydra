#ifndef _HYDRA_WATCH_
#define _HYDRA_WATCH_ true

#define HYDRA_SERVICE_WATCH 4
//out
#define HYDRA_PAYLOAD_WATCH_TYPE_BUTTON_PRESS 0
#define HYDRA_PAYLOAD_WATCH_TYPE_TIMEZONE_REQUEST 1
//in
#define HYDRA_PAYLOAD_WATCH_TYPE_TIMEZONE_RESPONSE 2
#define HYDRA_PAYLOAD_WATCH_TYPE_BRIGHT 3
#define HYDRA_PAYLOAD_WATCH_TYPE_DISPLAY 4
#define HYDRA_PAYLOAD_WATCH_TYPE_ALERT 5

#include "hydra.h"

struct HydraWatchConfig {
	union {
		uint8_t raw[2];
		struct {
			HydraAddress addr;
			uint8_t service;
		} parts;
	};
};

class HydraWatch: public HydraComponent {
	static const char* name;
	static const HydraConfigValueDescriptionList config_value_description_list;
	HydraWatchConfig config;
	int32_t timezone_offset;
	uint32_t timestamp;

public:
	virtual const char* getName();
	virtual const HydraConfigValueDescriptionList* getConfigDescription();
	virtual uint8_t* getConfig();
	virtual void init(Hydra* hydra);
	virtual bool writePacket(const HydraPacket* packet);
	virtual bool isPacketAvailable();
	virtual bool readPacket(HydraPacket* packet);
};

#endif
