#include "HydraWatch.h"
#include "lcd.h"

const char* HydraWatch::name = "Watch";

const HydraConfigValueDescriptionList HydraWatch::config_value_description_list = {
    1, 3, (HydraConfigValueDescription[]) {
        {HYDRA_CONFIG_VALUE_TYPE_ADDR_SERVICE, 3, "MasterServ"},
    }
};

const char* HydraWatch::getName() {
    return HydraWatch::name;
}

const HydraConfigValueDescriptionList* HydraWatch::getConfigDescription() {
    return & config_value_description_list;
}

uint8_t* HydraWatch::getConfig() {
    return this->config.raw;
}

void HydraWatch::init(Hydra* hydra) {
    hydra_debug("HydraWatch::init begin");
    HydraComponent::init(hydra);
    lcd_init();
    this->timestamp = hydra->getTime();
    hydra_debug("HydraWatch::init end");
}

bool HydraWatch::writePacket(const HydraPacket* packet) {
    hydra_debug("HydraWatch::writePacket");
    switch (packet->part.payload.type) {
    case HYDRA_WATCH_PAYLOAD_TYPE_DISPLAY:
        break;
    case HYDRA_WATCH_PAYLOAD_TYPE_BRIGHT:
        break;
    }
    return true;
}

bool HydraWatch::isPacketAvailable() {
    uint32_t timestamp = hydra->getTime();
    if (timestamp != this->timestamp) {
        uint32_t localtime = (timestamp + this->hydra->getTimeZoneOffset()) % 86400;
        uint8_t hour = localtime / 3600;
        uint8_t minute = (localtime % 3600) / 60;
        lcd_render_symbol(0, hour / 10);
        lcd_render_symbol(1, hour % 10);
        lcd_render_symbol(2, minute / 10);
        lcd_render_symbol(3, minute % 10);
        lcd_render_pixel(LCD_PIXEL_WATCH_DOT_HI, this->hydra->isTimeSynced() ? timestamp & 1 : 0);
        lcd_render_pixel(LCD_PIXEL_WATCH_DOT_LO, timestamp & 1);
    }
    this->timestamp = timestamp;

    return false;
}

bool HydraWatch::readPacket(HydraPacket* packet) {
    return false;
}
