#ifndef Arduino_h
    #define FROM_SCRATCH true
    #include <Arduino.h>
#else
    // Fix for compile with Arduino IDE
    #include <Ethernet.h>
    // this line also fix cmake error "SPI.h not found"
    #include <SPI.h>
    #include <RF24.h>
    #include <AES.h>
    #include <dht11.h>
    #include <EEPROM.h>
    #include <crc32.h>
#endif
#include <Hydra.h>
#include <HydraLan.h>
#include <HydraNrf.h>
#include <HydraCore.h>
#include <HydraEcho.h>
#include <HydraConfig.h>
#include <HydraDht11.h>

HydraLan lan = HydraLan();
HydraNrf nrf = HydraNrf(8, 7);
HydraCore core = HydraCore();
HydraEcho echo = HydraEcho();
HydraConfig config = HydraConfig();
HydraDht11 dht = HydraDht11(2);

HydraComponentDescriptionList components = {
    6, //total count
    2, //netif count
    4, //service count
    (HydraComponentDescription[]) {
        //network
        {{& lan, HYDRA_NET_SERVICE_ID}},
        {{& nrf, HYDRA_NET_SERVICE_ID}},
        //core components
        {{& core, HYDRA_CORE_SERVICE_ID}},
        {{& echo, HYDRA_ECHO_SERVICE_ID}},
        {{& config, HYDRA_CONFIG_SERVICE_ID}},
        //custom components
        {{& dht, HYDRA_SERVICE_ID_AUTO}}
    }
};

Hydra hydra = Hydra(& components);

void setup() {
    Serial.begin(115200);
    hydra.init();
}

void loop() {
    hydra.loop();
}

#if FROM_SCRATCH
int main(void) {
    init();
    setup();
    for (;;) {
        loop();
    }
    return 0;
}
#endif
