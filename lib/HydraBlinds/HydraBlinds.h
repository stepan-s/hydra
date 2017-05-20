#ifndef _HYDRA_BLINDS_
#define _HYDRA_BLINDS_ true

#define HYDRA_BLINDS_PAYLOAD(idx)                     HYDRA_PAYLOAD_TYPE_MIN + 0 + idx
#define HYDRA_BLINDS_PAYLOAD_TYPE_REQUEST_STATE        HYDRA_BLINDS_PAYLOAD(0)
#define HYDRA_BLINDS_PAYLOAD_TYPE_REPLY_STATE        HYDRA_BLINDS_PAYLOAD(1)
#define HYDRA_BLINDS_PAYLOAD_TYPE_COMMAND            HYDRA_BLINDS_PAYLOAD(2)

#define HYDRA_BLINDS_MOTOR_STATE_STOP 0
#define HYDRA_BLINDS_MOTOR_STATE_ROTATE_CW 1
#define HYDRA_BLINDS_MOTOR_STATE_ROTATE_CCW 2

#include <Hydra.h>
#include <AccelStepper.h>

struct HydraBlindsConfig {
    union {
        uint8_t raw[3];
        struct {
            uint16_t speed;
            uint8_t threshold;
        } parts;
    };
};

class HydraBlinds: public HydraComponent {
    static const char* name;
    static const HydraConfigValueDescriptionList config_value_description_list;
    HydraBlindsConfig config;
    bool reply_ready;
    HydraAddress reply_to_address;
    uint8_t reply_to_service;
    AccelStepper* motor;
    uint8_t sensor_pin;
    uint8_t motor_state;
    int8_t position;
    int8_t position_dest;
    void loop();

public:
    HydraBlinds(uint8_t sensorPin, uint8_t motorPin1, uint8_t motorPin2, uint8_t motorPin3, uint8_t motorPin4);
    virtual const char* getName();
    virtual const HydraConfigValueDescriptionList* getConfigDescription();
    virtual uint8_t* getConfig();
    virtual void init(Hydra* hydra);
    virtual bool writePacket(const HydraPacket* packet);
    virtual bool isPacketAvailable();
    virtual bool readPacket(HydraPacket* packet);
};

#endif
