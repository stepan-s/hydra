#ifndef _HYDRA_CORE_
#define _HYDRA_CORE_ true

#define HYDRA_SERVICE_CORE 1
#define HYDRA_CORE_PAYLOAD_TYPE_SET_TIME 0
#define HYDRA_CORE_PAYLOAD_TYPE_REBOOT 1

#include "hydra.h"

class HydraCore: public HydraComponent {
	static const char* name;

public:
	virtual const char* getName();
	virtual bool writePacket(const HydraPacket* packet);
};

#endif
