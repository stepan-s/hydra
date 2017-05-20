#ifndef _HYDRA_DHT11_
#define _HYDRA_DHT11_ true

#define HYDRA_DHT11_PAYLOAD(idx)                 HYDRA_PAYLOAD_TYPE_MIN + 0 + idx
#define HYDRA_DHT11_PAYLOAD_TYPE_REQUEST        HYDRA_DHT11_PAYLOAD(0)
#define HYDRA_DHT11_PAYLOAD_TYPE_REPLY            HYDRA_DHT11_PAYLOAD(1)

#include <Hydra.h>
#include <dht11.h>

struct HydraDht11Config {};

class HydraDht11: public HydraComponent {
    static const char* name;
    uint8_t pin;
    bool reply_ready;
    HydraAddress reply_to_address;
    uint16_t reply_to_service;
    dht11 sensor;

public:
    HydraDht11(uint8_t pin);
    virtual const char* getName();
    virtual void init(Hydra* hydra);
    virtual bool writePacket(const HydraPacket* packet);
    virtual bool isPacketAvailable();
    virtual bool readPacket(HydraPacket* packet);
};

#endif
