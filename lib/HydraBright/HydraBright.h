#ifndef _HYDRA_BRIGHT_
#define _HYDRA_BRIGHT_ true

#define HYDRA_BRIGHT_PAYLOAD(idx)                HYDRA_PAYLOAD_TYPE_MIN + 0 + idx
#define HYDRA_BRIGHT_PAYLOAD_TYPE_REQUEST        HYDRA_BRIGHT_PAYLOAD(0)
#define HYDRA_BRIGHT_PAYLOAD_TYPE_REPLY          HYDRA_BRIGHT_PAYLOAD(1)

#include <Hydra.h>

struct HydraBrightConfig {
    union {
        uint8_t raw[2];
        struct {
            uint8_t gain_min;
            uint8_t gain_max;
        } parts;
    };
};

class HydraBright: public HydraComponent {
    static const char* name;
    static const HydraConfigValueDescriptionList config_value_description_list;
    HydraBrightConfig config;
    uint8_t pin;
    bool reply_ready;
    HydraAddressPort reply_to;

public:
    HydraBright(uint8_t pin);
    virtual const HydraConfigValueDescriptionList* getConfigDescription();
    virtual uint8_t* getConfig();
    virtual const char* getName();
    virtual bool writePacket(const HydraPacket* packet);
    virtual bool isPacketAvailable();
    virtual bool readPacket(HydraPacket* packet);
};

#endif
