#ifndef _HYDRA_WATCH_LM16x8_
#define _HYDRA_WATCH_LM16x8_ true

#define HYDRA_WATCH_LM16x8_PAYLOAD(idx)                HYDRA_PAYLOAD_TYPE_MIN + 0 + idx
#define HYDRA_WATCH_LM16x8_PAYLOAD_TYPE_BRIGHT         HYDRA_WATCH_LM16x8_PAYLOAD(3)
#define HYDRA_WATCH_LM16x8_PAYLOAD_TYPE_DISPLAY        HYDRA_WATCH_LM16x8_PAYLOAD(4)

#include <Hydra.h>
#include <LedControl.h>

struct HydraWatchLM16x8Config {
    union {
        uint8_t raw[2];
        struct {
            HydraAddress addr;
            uint8_t service;
        } parts;
    };
};

class HydraWatchLM16x8: public HydraComponent {
    static const char* name;
    static const HydraConfigValueDescriptionList config_value_description_list;
    HydraWatchLM16x8Config config;
    uint32_t timestamp;

    LedControl* led_control;
    byte display_values[4] = {0, 0, 0, 0};
    byte point_stage = 4;
    HydraTimeout point_timeout;
    void displayInit();
    void setDigit(byte index, byte digit, boolean force = false);
    void setDisplay(int value, boolean force = false);
    void setPoint(byte stage);

public:
    HydraWatchLM16x8(uint8_t dataPin, uint8_t clkPin, uint8_t csPin);
    virtual const char* getName();
    virtual const HydraConfigValueDescriptionList* getConfigDescription();
    virtual uint8_t* getConfig();
    virtual void init(Hydra* hydra);
    virtual bool writePacket(const HydraPacket* packet);
    virtual bool isPacketAvailable();
    virtual bool readPacket(HydraPacket* packet);
};

#endif
