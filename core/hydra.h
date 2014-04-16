#ifndef _HYDRA_
#define _HYDRA_ true

#include "EEPROM.h"
#include <WString.h>

#define HYDRA_PACKET_SIZE 32
#define HYDRA_PACKET_PAYLOAD_DATA_SIZE 20

#define HYDRA_ADDR_BROADCAST_ALL 0xFFFF
#define HYDRA_ADDR_BROADCAST_NET 0x00FF
#define HYDRA_ADDR_BROADCAST_DEVICES 0xFF00
#define HYDRA_ADDR_NULL 0x0000
#define HYDRA_ADDR_LOCAL 0x0001

#define HYDRA_NET_BROADCAST 0xFF
#define HYDRA_NET_NULL 0x00

#define HYDRA_DEVICE_BROADCAST 0xFF
#define HYDRA_DEVICE_NULL 0x00

#define HYDRA_SERVICE_NET 0

#define hydra_is_addr_to_all(addr) ((addr.raw & HYDRA_ADDR_BROADCAST_ALL) == HYDRA_ADDR_BROADCAST_ALL)
#define hydra_is_addr_to_net(addr) ((addr.raw & HYDRA_ADDR_BROADCAST_NET) == HYDRA_ADDR_BROADCAST_NET)
#define hydra_is_addr_to_devices(addr) ((addr.raw & HYDRA_ADDR_BROADCAST_DEVICES) == HYDRA_ADDR_BROADCAST_DEVICES)
#define hydra_is_addr_local(addr) (addr.raw == HYDRA_ADDR_LOCAL)
#define hydra_is_addr_null(addr) (addr.raw == HYDRA_ADDR_NULL)

#ifndef HYDRA_BOOT_CONSOLE_WAIT_TIME
	#define HYDRA_BOOT_CONSOLE_WAIT_TIME 3
#endif

#define hydra_debug_(msg)					Serial.println(msg)
#define hydra_debug_param_(msg, param)		Serial.print(msg); Serial.println(param)
#ifdef HYDRA_DEBUG
	#define hydra_debug(msg)					Serial.println(msg)
	#define hydra_debug_param(msg, param)		/*Serial.print(msg);*/ Serial.println(param)
#else
	#define hydra_debug(msg)
	#define hydra_debug_param(msg, param)
#endif

union HydraAddress {
	uint16_t raw;
	struct {
		uint8_t device;
		uint8_t net;
	} part;
};

struct HydraAddressPort {
	HydraAddress addr;
	uint8_t service;
};

struct HydraPacket {
	union {
		uint8_t data[HYDRA_PACKET_SIZE];
		struct {
			HydraAddress	from_addr;
			HydraAddress	to_addr;
			uint8_t			from_service;
			uint8_t			to_service;
			uint32_t		timestamp;
			union {
				uint8_t		payload_raw[HYDRA_PACKET_PAYLOAD_DATA_SIZE + 2];
				struct {
					uint16_t	type;
					uint8_t		data[HYDRA_PACKET_PAYLOAD_DATA_SIZE];
				} payload;
			};
		} part;
	};
};

struct HydraConfigValueDescription {
	const uint8_t size;
	const char* caption;
};

struct HydraConfigValueDescriptionList {
	const uint8_t count;
	const uint8_t size;
	const HydraConfigValueDescription* list;
};

class Hydra;

class HydraComponent {
public:
	Hydra* hydra;
	static const char* name;
	static const HydraConfigValueDescriptionList config_value_description_list;

	virtual const char* getName();
	virtual const HydraConfigValueDescriptionList* getConfigDescription();
	virtual uint8_t* getConfig();
	virtual void init(Hydra* hydra);
	virtual bool writePacket(const HydraPacket* packet);
	virtual bool isPacketAvailable();
	virtual bool readPacket(HydraPacket* packet);
	//virtual ~HydraComponent();
};

class HydraNetComponent: public HydraComponent {
public:
	virtual bool writePacket(const HydraPacket* packet);
	virtual HydraAddress getGateway(const HydraAddress destionation);
	virtual bool sendPacket(const HydraAddress to, const HydraPacket* packet, const bool set_from_addr);
	virtual HydraAddress getAddress();
};

union HydraComponentDescription {
	struct {
		HydraComponent* component;
		uint8_t id;
	};
	struct {
		HydraNetComponent* component;
	} netif;
	struct {
		HydraComponent* component;
		uint8_t id;
	} service;
};

struct HydraComponentDescriptionList {
	const uint8_t totalCount;
	const uint8_t netifCount;
	const uint8_t serviceCount;
	HydraComponentDescription* list;
};

class Hydra {
	HydraComponentDescriptionList* components;
	uint32_t ms;
	uint32_t timestamp;
	int32_t timestamp_last;
	int32_t timezone_offset_seconds;
	HydraAddress default_gateway;
	void consoleRun();
	void consoleHelp();
	void consolePrintConfig();
	void consolePrintHex(uint8_t value);
	void consolePrintHex(uint8_t *buffer, int length);
	String consoleGetConfigValueFullName(uint8_t index, uint8_t value_index);
	bool consoleSetConfigValue(String name, String value);
	bool consoleParseHex(String hex, uint8_t* buffer, uint8_t length);
	void loadConfig();
	void saveConfig();
	void updateTimer();

public:
	Hydra(HydraComponentDescriptionList* components);
	void bootConsole();
	void init();
	void loop();
	void route(const HydraPacket* packet, const HydraAddress received_via);
	void landing(const HydraPacket* packet, const HydraAddress received_via, const HydraAddress landing_via);
	uint32_t getTime();
	bool isTimeSynced();
	int32_t getTimeZoneOffset();
	void setTime(uint32_t timestamp, int16_t timezone_offset_minutes = 32768);
	HydraAddress getDefaultGateway();
	bool isMasterOnline();
};

class HydraTimeout {
	static uint16_t last;
	static uint16_t delta;
	uint16_t left;
public:
	void begin(uint16_t ms);
	void tick();
	bool isEnd();
	void static calcDelta();
};

#endif
