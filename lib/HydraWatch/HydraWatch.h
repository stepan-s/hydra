#ifndef _HYDRA_WATCH_
#define _HYDRA_WATCH_ true

#define HYDRA_WATCH_PAYLOAD(idx)				 HYDRA_PAYLOAD_TYPE_MIN + 0 + idx
#define HYDRA_WATCH_PAYLOAD_TYPE_BRIGHT			HYDRA_WATCH_PAYLOAD(3)
#define HYDRA_WATCH_PAYLOAD_TYPE_DISPLAY		HYDRA_WATCH_PAYLOAD(4)

#include "Hydra.h"

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
