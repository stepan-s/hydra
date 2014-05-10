#ifndef _HYDRA_CORE_
#define _HYDRA_CORE_ true

#define HYDRA_CORE_SERVICE_ID 1
#define HYDRA_CORE_PAYLOAD_TYPE_SET_TIME 0
#define HYDRA_CORE_PAYLOAD_TYPE_REBOOT 1
#define HYDRA_CORE_PAYLOAD_TYPE_ENUM_REQUEST 2
#define HYDRA_CORE_PAYLOAD_TYPE_ENUM_REPLY 3

#define HYDRA_CORE_ENUM_DELAY_MS 20

#include "hydra.h"

class HydraCore: public HydraComponent {
	static const char* name;
	bool online;
	HydraAddressPort reply_to;
	HydraTimeout reply_timeout;

public:
	HydraCore();
	virtual const char* getName();
	virtual bool isPacketAvailable();
	virtual bool writePacket(const HydraPacket* packet);
	virtual bool readPacket(HydraPacket* packet);
};

#endif
