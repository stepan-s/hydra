#include "hydra_watch.h"
#include "lcd.h"
#include <Arduino.h>

const char* HydraWatch::name = "Watch";

const HydraConfigValueDescriptionList HydraWatch::config_value_description_list = {
	2, 3, (HydraConfigValueDescription[]) {
		{2, "MasterADDR"},
		{1, "MasterService"},
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
	case HYDRA_PAYLOAD_WATCH_TYPE_TIMEZONE_RESPONSE:
		this->timezone_offset = ((int32_t*)packet->part.payload.data)[0];
		break;
	case HYDRA_PAYLOAD_WATCH_TYPE_ALERT:
		break;
	case HYDRA_PAYLOAD_WATCH_TYPE_DISPLAY:
		break;
	case HYDRA_PAYLOAD_WATCH_TYPE_BRIGHT:
		break;
	}
	return true;
}

bool HydraWatch::isPacketAvailable() {
	uint32_t timestamp = hydra->getTime();
	if (timestamp != this->timestamp) {
		uint32_t localtime = (timestamp + this->timezone_offset) % 86400;
		uint8_t hour = localtime / 3600;
		uint8_t minute = (localtime % 3600) / 60;
	    lcd_render_symbol(0, hour / 10);
	    lcd_render_symbol(1, hour % 10);
	    lcd_render_symbol(2, minute / 10);
	    lcd_render_symbol(3, minute % 10);
	    lcd_render_pixel(LCD_PIXEL_WATCH_DOT_HI, timestamp & 1);
	    lcd_render_pixel(LCD_PIXEL_WATCH_DOT_LO, timestamp & 1);
	}
	this->timestamp = timestamp;

	return false;
}

bool HydraWatch::readPacket(HydraPacket* packet) {
	return false;
}
