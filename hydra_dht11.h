#ifndef _HYDRA_DHT11_
#define _HYDRA_DHT11_ true

#define HYDRA_SERVICE_DHT11 3
#define HYDRA_PAYLOAD_DHT11_TYPE_REQUEST 0
#define HYDRA_PAYLOAD_DHT11_TYPE_REPLY 1

#include "hydra.h"
#include <dht11.h>

struct HydraDht11Config {};

class HydraDht11: public HydraComponent {
	static const char* name;
	static const HydraConfigValueDescriptionList config_value_description_list;
	HydraDht11Config config;
	uint8_t pin;
	bool reply_ready;
	HydraAddress reply_to_address;
	uint16_t reply_to_service;
	dht11 sensor;

public:
	HydraDht11(uint8_t pin);
	virtual const char* getName();
	virtual const HydraConfigValueDescriptionList* getConfigDescription();
	virtual uint8_t* getConfig();
	virtual void init(Hydra* hydra);
	virtual bool writePacket(const HydraPacket* packet);
	virtual bool isPacketAvailable();
	virtual bool readPacket(HydraPacket* packet);
};

#endif
