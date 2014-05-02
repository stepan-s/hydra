#include "hydra_core.h"
#include <Arduino.h>
#include <avr/wdt.h>

const char* HydraCore::name = "Core";

const char* HydraCore::getName() {
	return HydraCore::name;
}

HydraCore::HydraCore() {
	this->online = false;
}

bool HydraCore::isPacketAvailable() {
	bool online = this->hydra->isMasterOnline();
	if (online != this->online) {
		if (online) {
			hydra_fprintln("ONLINE");
		} else {
			hydra_fprintln("OFFLINE");
		}
		this->online = online;
	}
	return false;
}

bool HydraCore::writePacket(const HydraPacket* packet) {
	hydra_debug("HydraCore::writePacket");
	switch (packet->part.payload.type) {
	case HYDRA_CORE_PAYLOAD_TYPE_SET_TIME:
		{
			int16_t timezone_offset_minutes = *((int16_t*)packet->part.payload.data);
			hydra_debug_param("HydraCore::writePacket setTime, ts: ", packet->part.timestamp);
			hydra_debug_param("HydraCore::writePacket setTime, tz: ", timezone_offset_minutes);
			this->hydra->setTime(packet->part.timestamp, timezone_offset_minutes);
			this->hydra->master_online_timeout.begin(HYDRA_MASTER_ONLINE_TIMEOUT);
			this->hydra->time_sync_timeout.begin(HYDRA_TIME_SYNC_TIMEOUT);
		}
		break;
	case HYDRA_CORE_PAYLOAD_TYPE_REBOOT:
		hydra_fprintln("REBOOT");
		cli();                  // Clear interrupts
		wdt_enable(WDTO_15MS);  // Set the Watchdog to 15ms
		while(1);               // Enter an infinite loop
		break;
	default:
		return false;
	}
	return true;
}
