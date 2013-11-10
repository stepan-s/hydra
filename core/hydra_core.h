#ifndef _HYDRA_CORE_
#define _HYDRA_CORE_ true

#define HYDRA_SERVICE_CORE 1
#define HYDRA_PAYLOAD_CORE_TYPE_SET_TIME 0

#include "hydra.h"

struct HydraCoreConfig {};

class HydraCore: public HydraComponent {
	static const char* name;
	static const HydraConfigValueDescriptionList config_value_description_list;
	HydraCoreConfig config;

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
