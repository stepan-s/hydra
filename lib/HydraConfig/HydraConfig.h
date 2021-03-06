#ifndef _HYDRA_CONFIG_
#define _HYDRA_CONFIG_ true

#include <Hydra.h>

#define HYDRA_CONFIG_SERVICE_ID 3
#define HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_COUNT 1
#define HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_COUNT 2
#define HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_ID 3
#define HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_ID 4
#define HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_NAME 5
#define HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_NAME 6
#define HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_CONFIG_COUNT 7
#define HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_CONFIG_COUNT 8
#define HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_CONFIG_NAME 9
#define HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_CONFIG_NAME 10
#define HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_CONFIG_GET_VALUE 11
#define HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_CONFIG_GET_VALUE 12
#define HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_SERVICE_CONFIG_SET_VALUE 13
#define HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_SERVICE_CONFIG_SET_VALUE 14
#define HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_CONFIG_READ 15
#define HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_CONFIG_READ 16
#define HYDRA_CONFIG_PAYLOAD_TYPE_REQUEST_CONFIG_WRITE 17
#define HYDRA_CONFIG_PAYLOAD_TYPE_REPLY_CONFIG_WRITE 18

class HydraConfig: public HydraComponent {
    static const char* name;
    HydraAddressPort reply_to;
    uint8_t reply_type;
    uint8_t service_index;
    uint8_t value_index;
    uint8_t request_offset;
    uint8_t request_size;
    virtual bool getConfigValuePointer(const uint8_t service_index, const uint8_t value_index, uint8_t** out_pointer, uint8_t* out_size);

public:
    virtual const char* getName();
    virtual void init(Hydra* hydra);
    virtual bool writePacket(const HydraPacket* packet);
    virtual bool isPacketAvailable();
    virtual bool readPacket(HydraPacket* packet);
};

#endif
