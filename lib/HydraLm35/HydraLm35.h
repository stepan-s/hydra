#ifndef _HYDRA_LM35_
#define _HYDRA_LM35_ true

#define HYDRA_LM35_PAYLOAD(idx)                HYDRA_PAYLOAD_TYPE_MIN + 0 + idx
#define HYDRA_LM35_PAYLOAD_TYPE_REQUEST        HYDRA_LM35_PAYLOAD(0)
#define HYDRA_LM35_PAYLOAD_TYPE_REPLY          HYDRA_LM35_PAYLOAD(1)

#include <Hydra.h>

class HydraLm35: public HydraComponent {
    static const char* name;
    uint8_t pin;
    bool reply_ready;
    HydraAddressPort reply_to;

public:
    HydraLm35(uint8_t pin);
    virtual const char* getName();
    virtual void init(Hydra* hydra);
    virtual bool writePacket(const HydraPacket* packet);
    virtual bool isPacketAvailable();
    virtual bool readPacket(HydraPacket* packet);
};

#endif
