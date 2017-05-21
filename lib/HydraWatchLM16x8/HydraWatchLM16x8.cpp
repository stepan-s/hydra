#include "HydraWatchLM16x8.h"

const byte font[10][8] = {
    {
        B010,
        B101,
        B101,
        B101,
        B101,
        B101,
        B101,
        B010
    },
    {
        B010,
        B011,
        B010,
        B010,
        B010,
        B010,
        B010,
        B111
    },
    {
        B011,
        B100,
        B100,
        B100,
        B010,
        B001,
        B001,
        B111
    },
    {
        B011,
        B100,
        B100,
        B010,
        B100,
        B100,
        B100,
        B011
    },
    {
        B001,
        B101,
        B101,
        B111,
        B100,
        B100,
        B100,
        B100
    },
    {
        B111,
        B001,
        B001,
        B011,
        B100,
        B100,
        B100,
        B011
    },
    {
        B110,
        B001,
        B001,
        B011,
        B101,
        B101,
        B101,
        B010
    },
    {
        B111,
        B100,
        B100,
        B100,
        B100,
        B010,
        B010,
        B010
    },
    {
        B010,
        B101,
        B101,
        B010,
        B101,
        B101,
        B101,
        B010
    },
    {
        B010,
        B101,
        B101,
        B101,
        B110,
        B100,
        B100,
        B011
    },
};

HydraWatchLM16x8::HydraWatchLM16x8(uint8_t dataPin, uint8_t clkPin, uint8_t csPin) {
    this->led_control = new LedControl(dataPin, clkPin, csPin, 2);
}

const char* HydraWatchLM16x8::name = "Watch";

const HydraConfigValueDescriptionList HydraWatchLM16x8::config_value_description_list = {
    1, 3, (HydraConfigValueDescription[]) {
        {HYDRA_CONFIG_VALUE_TYPE_ADDR_SERVICE, 3, "MasterServ"},
    }
};

const char* HydraWatchLM16x8::getName() {
    return HydraWatchLM16x8::name;
}

const HydraConfigValueDescriptionList* HydraWatchLM16x8::getConfigDescription() {
    return & config_value_description_list;
}

uint8_t* HydraWatchLM16x8::getConfig() {
    return this->config.raw;
}

void HydraWatchLM16x8::init(Hydra* hydra) {
    hydra_debug("HydraWatchLM16x8::init begin");
    HydraComponent::init(hydra);
    this->displayInit();
    this->timestamp = hydra->getTime();
    hydra_debug("HydraWatchLM16x8::init end");
}

bool HydraWatchLM16x8::writePacket(const HydraPacket* packet) {
    hydra_debug("HydraWatchLM16x8::writePacket");
    switch (packet->part.payload.type) {
    case HYDRA_WATCH_LM16x8_PAYLOAD_TYPE_DISPLAY:
        break;
    case HYDRA_WATCH_LM16x8_PAYLOAD_TYPE_BRIGHT:
        int devices = this->led_control->getDeviceCount();
        for (int address = 0; address < devices; address++) {
            this->led_control->setIntensity(address, packet->part.payload.data[0]);
        }
        break;
    }
    return true;
}

bool HydraWatchLM16x8::isPacketAvailable() {
    uint32_t timestamp = hydra->getTime();
    if (timestamp != this->timestamp) {
        uint32_t localtime = (timestamp + this->hydra->getTimeZoneOffset()) % 86400;
        uint8_t hour = localtime / 3600;
        uint8_t minute = (localtime % 3600) / 60;
        this->setDigit(0, hour / 10);
        this->setDigit(1, hour % 10);
        this->setDigit(2, minute / 10);
        this->setDigit(3, minute % 10);
        int point_stage;
        if (this->hydra->isTimeSynced()) {
            point_stage = (timestamp & 1) ? 4 : 5;
        } else {
            point_stage = timestamp & 3;
        }
        this->setPoint(point_stage);
    }
    this->timestamp = timestamp;

    return false;
}

bool HydraWatchLM16x8::readPacket(HydraPacket* packet) {
    return false;
}

// Display

void HydraWatchLM16x8::displayInit() {
    int devices = this->led_control->getDeviceCount();
    for (int address = 0; address < devices; address++) {
        this->led_control->shutdown(address, false);
        this->led_control->setIntensity(address, 8);
        this->led_control->clearDisplay(address);
    }
    this->setDisplay(0, true);
}

void HydraWatchLM16x8::setDigit(int index, int digit, boolean force) {
    if (force || (this->display_values[index] != digit)) {
        this->display_values[index] = digit;
        int address = index >> 1;
        int a = this->display_values[index | B01];
        int b = this->display_values[index & B10];
        int a_shift = address ? 5 : 4;
        int b_shift = address ? 1 : 0;

        for (int row = 0; row < 8; row++) {
            int value = (font[a][row] << a_shift) | (font[b][row] << b_shift);
            this->led_control->setRow(address, 7 - row, value);
        }
    }
}

void HydraWatchLM16x8::setDisplay(int value, boolean force) {
    int a = value % 10;
    int b = (value / 10) % 10;
    int c = (value / 100) % 10;
    int d = (value / 1000) % 10;
    this->setDigit(0, d, force);
    this->setDigit(1, c, force);
    this->setDigit(2, b, force);
    this->setDigit(3, a, force);
}

void HydraWatchLM16x8::setPoint(int stage) {
    int pixels;
    switch (stage) {
        case 0:
            pixels = B1001;
            break;
        case 1:
            pixels = B0011;
            break;
        case 2:
            pixels = B0110;
            break;
        case 3:
            pixels = B1100;
            break;
        case 4:
            pixels = B0000;
            break;
        case 5:
            pixels = B1111;
            break;
    }
    this->led_control->setLed(0, 3, 0, pixels & B0001);   // left bottom
    this->led_control->setLed(0, 4, 0, pixels & B0010);   // left top
    this->led_control->setLed(1, 4, 7, pixels & B0100);   // right top
    this->led_control->setLed(1, 3, 7, pixels & B1000);   // right bottom
}
