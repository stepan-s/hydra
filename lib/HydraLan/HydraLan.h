#ifndef _HYDRA_LAN_
#define _HYDRA_LAN_ true

#include "Hydra.h"
#include <Ethernet.h>
#include <EthernetUdp.h>

struct HydraLanConfig : HydraNetConfig {
	union {
		uint8_t raw[14 + HYDRA_NET_ROUTE_COUNT * 2];
		struct {
			HydraAddress addr;
			NydraNetRoute routes[HYDRA_NET_ROUTE_COUNT];
			uint8_t mac[6];
			uint8_t ip[4];
			uint16_t port;
		} parts;
	};
};

class HydraLan: public HydraNetComponent {
	static const char* name;
	static const HydraConfigValueDescriptionList config_value_description_list;
	HydraLanConfig config;
	EthernetUDP udp;

public:
	HydraLan();
	virtual const char* getName();
	virtual const HydraConfigValueDescriptionList* getConfigDescription();
	virtual uint8_t* getConfig();
	virtual void init(Hydra* hydra);
	virtual bool isPacketAvailable();
	virtual bool readPacket(HydraPacket* packet);
	virtual bool sendPacket(const HydraAddress to, const HydraPacket* packet);
};

#endif
