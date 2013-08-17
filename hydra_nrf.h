#ifndef _HYDRA_NRF_
#define _HYDRA_NRF_ true

#include "hydra.h"
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define HYDRA_NRF_BC 0xFF

struct HydraNrfConfig {
	union {
		uint8_t raw[22];
		struct {
			HydraAddress addr;
			uint8_t net[3];
			uint8_t channel;
			uint8_t enc_key[16];
		} parts;
	};
};

class HydraNrf: public HydraNetComponent {
	static const char* name;
	static const HydraConfigValueDescriptionList config_value_description_list;
	HydraNrfConfig config;
	RF24* radio;
	uint8_t enc_iv[16];

public:
	HydraNrf(uint8_t cePin, uint8_t csPin);
	virtual const char* getName();
	virtual const HydraConfigValueDescriptionList* getConfigDescription();
	virtual uint8_t* getConfig();
	virtual void init(Hydra* hydra);
	virtual bool isPacketAvailable();
	virtual bool readPacket(HydraPacket* packet);
	virtual HydraAddress getGateway(HydraAddress destionation);
	virtual bool sendPacket(const HydraAddress to, const HydraPacket* packet, const bool set_from_addr);
	virtual HydraAddress getAddress();
};

#endif
