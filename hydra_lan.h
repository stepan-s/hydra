#ifndef _HYDRA_LAN_
#define _HYDRA_LAN_ true

#include "hydra.h"
#include <Ethernet.h>
#include <EthernetUdp.h>

struct HydraLanConfig {
	union {
		uint8_t raw[20];
		struct {
			HydraAddress addr;
			uint8_t mac[6];
			uint8_t local_ip[4];
			uint16_t local_port;
			uint8_t gateway_ip[4];
			uint16_t gateway_port;
		} parts;
	};
};

class HydraLan: public HydraNetComponent {
	static const char* name;
	static const HydraConfigValueDescriptionList config_value_description_list;
	HydraLanConfig config;
	EthernetUDP udp;
	IPAddress gateway_ip;

public:
	HydraLan();
	virtual const char* getName();
	virtual const HydraConfigValueDescriptionList* getConfigDescription();
	virtual uint8_t* getConfig();
	virtual void init(Hydra* hydra);
	virtual bool isPacketAvailable();
	virtual bool readPacket(HydraPacket* packet);
	virtual HydraAddress getGateway(const HydraAddress destionation);
	virtual bool sendPacket(const HydraAddress to, const HydraPacket* packet, const bool set_from_addr);
	virtual HydraAddress getAddress();
};

#endif
