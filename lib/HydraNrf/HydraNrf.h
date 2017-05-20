#ifndef _HYDRA_NRF_
#define _HYDRA_NRF_ true

#include <Hydra.h>
#include <RF24.h>
#include <AES.h>

#define HYDRA_NRF_BC 0xFF

struct HydraNrfConfig : HydraNetConfig {
    union {
        uint8_t raw[24 + HYDRA_NET_ROUTE_COUNT * 2];
        struct {
            HydraAddress addr;
            NydraNetRoute routes[HYDRA_NET_ROUTE_COUNT];
            uint8_t net[3];
            uint8_t channel;
            uint8_t enc_key[16];
            struct {
                uint8_t retries_delay: 4;
                uint8_t retries_count: 4;
                uint8_t speed: 2;
                uint8_t power: 2;
                uint8_t crc: 2;
                uint8_t auto_ack: 1;
            } radio_opts;
        } parts;
    };
};

class HydraNrf: public HydraNetComponent {
    static const char* name;
    static const HydraConfigValueDescriptionList config_value_description_list;
    HydraNrfConfig config;
    RF24* radio;
    uint8_t enc_iv[16];
    AES* aes;

public:
    HydraNrf(uint8_t cePin, uint8_t csPin);
    virtual const char* getName();
    virtual const HydraConfigValueDescriptionList* getConfigDescription();
    virtual uint8_t* getConfig();
    virtual void init(Hydra* hydra);
    virtual bool isPacketAvailable();
    virtual bool readPacket(HydraPacket* packet);
    virtual bool sendPacket(const HydraAddress to, const HydraPacket* packet);
};

#endif
