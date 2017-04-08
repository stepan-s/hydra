#include <Arduino.h>
#include "hydra.h"
#include "hydra_core.h"
#include "hydra_lan.h"
#include "hydra_nrf.h"
#include "hydra_echo.h"
#include "hydra_dht11.h"

HydraLan lan = HydraLan();
HydraNrf nrf = HydraNrf(8, 7);
HydraCore core = HydraCore();
HydraEcho echo = HydraEcho();
HydraDht11 dht = HydraDht11(2);

HydraComponentDescriptionList components = {
    5, //total count
    2, //netif count
    3, //service count
    (HydraComponentDescription[]) {
        {{& lan, HYDRA_SERVICE_NET}},
        {{& nrf, HYDRA_SERVICE_NET}},
        {{& core, HYDRA_SERVICE_CORE}},
        {{& echo, HYDRA_SERVICE_ECHO}},
        {{& dht, HYDRA_SERVICE_DHT11}}
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

int main(void) {
    init();
    setup();
    for (;;) {
        loop();
    }
    return 0;
}
