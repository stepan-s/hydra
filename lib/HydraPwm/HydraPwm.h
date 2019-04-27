#ifndef _HYDRA_PWM_
#define _HYDRA_PWM_ true

#define HYDRA_PWM_PAYLOAD(idx)                      HYDRA_PAYLOAD_TYPE_MIN + 0 + idx
#define HYDRA_PWM_PAYLOAD_TYPE_REQUEST_STATE        HYDRA_PWM_PAYLOAD(0)
#define HYDRA_PWM_PAYLOAD_TYPE_REPLY_STATE          HYDRA_PWM_PAYLOAD(1)
#define HYDRA_PWM_PAYLOAD_TYPE_COMMAND              HYDRA_PWM_PAYLOAD(2)

#include <Hydra.h>

class HydraPwm: public HydraComponent {
    static const char* name;
    bool reply_ready;
    HydraAddress reply_to_address;
    uint8_t reply_to_service;

    uint8_t out_pin;
    uint8_t value;
    void loop();

public:
    HydraPwm(uint8_t out_pin1);
    virtual const char* getName();
    virtual void init(Hydra* hydra);
    virtual bool writePacket(const HydraPacket* packet);
    virtual bool isPacketAvailable();
    virtual bool readPacket(HydraPacket* packet);
};

#endif
