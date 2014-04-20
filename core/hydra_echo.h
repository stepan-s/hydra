#ifndef _HYDRA_ECHO_
#define _HYDRA_ECHO_ true

#define HYDRA_SERVICE_ECHO 2
#define HYDRA_ECHO_PAYLOAD_TYPE_REQUEST 0
#define HYDRA_ECHO_PAYLOAD_TYPE_REPLY 1

#include "hydra.h"

struct HydraEchoConfig {
	union {
		uint8_t raw[2];
		struct {
			HydraAddress addr;
		} parts;
	};
};

class HydraEcho: public HydraComponent {
	static const char* name;
	static const HydraConfigValueDescriptionList config_value_description_list;
	HydraEchoConfig config;
	bool reply_ready;
	HydraAddress reply_from_address;
	HydraAddress reply_to_address;
	uint16_t reply_to_service;
	uint8_t reply_payload[HYDRA_PACKET_PAYLOAD_DATA_SIZE];
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
