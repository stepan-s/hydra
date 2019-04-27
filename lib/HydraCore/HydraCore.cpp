#include "HydraCore.h"
#include <avr/wdt.h>
#include <crc32.h>

const char* HydraCore::name = "Core";

const char* HydraCore::getName() {
    return HydraCore::name;
}

HydraCore::HydraCore() {
    this->online = false;
    this->reply_to = (HydraAddressPort){{HYDRA_ADDR_NULL}, 0};
    this->reply_timeout.begin(0);
}

bool HydraCore::isPacketAvailable() {
    this->reply_timeout.tick();
    bool online = this->hydra->isMasterOnline();
    if (online != this->online) {
        if (online) {
            hydra_fprintln("ONLINE");
        } else {
            hydra_fprintln("OFFLINE");
        }
        this->online = online;
    }
    if (!hydra_is_addr_null(this->reply_to.addr) && this->reply_timeout.isEnd()) {
        return true;
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
    case HYDRA_CORE_PAYLOAD_TYPE_ENUM_REQUEST:
        if ((packet->part.to_addr.part.device & packet->part.payload.data[1]) == packet->part.payload.data[0]) {
            hydra_fprint("ENUM REQ: ");
            hydra_hprintln(packet->part.from_addr.raw);
            this->reply_to.addr = packet->part.from_addr;
            this->reply_to.service = packet->part.from_service;
            //delay depend device number
            this->reply_timeout.begin(packet->part.to_addr.part.device * (uint16_t)HYDRA_CORE_ENUM_DELAY_MS);
        }
        break;
    default:
        return false;
    }
    return true;
}

bool HydraCore::readPacket(HydraPacket* packet) {
    if (!hydra_is_addr_null(this->reply_to.addr)) {
        packet->part.to_addr = this->reply_to.addr;
        packet->part.to_service = this->reply_to.service;
        packet->part.payload.type = HYDRA_CORE_PAYLOAD_TYPE_ENUM_REPLY;
        this->reply_to.addr = (HydraAddress){HYDRA_ADDR_NULL};

        Crc32 hard = Crc32();
        Crc32 soft = Crc32();
        hard.update(HARDWARE_FREQ);
        hard.update(HARDWARE_ID);

        HydraComponentDescriptionList* components = this->hydra->components;
        hard.update(components->netifCount);
        hard.update(components->serviceCount);

        uint8_t i;
        for(i = 0; i < components->totalCount; ++i) {
            const HydraComponentDescription* item = & components->list[i];

            hard.update(i);
            hard.update((char *) item->component->getName());
            if (i >= components->netifCount) {
                hard.update(item->id);
            }

            const HydraConfigValueDescriptionList* config_description = item->component->getConfigDescription();
            if (config_description) {
                hard.update(config_description->count);
                uint8_t j;
                for(j = 0; j < config_description->count; ++j) {
                    hard.update(j);
                    hard.update(config_description->list[j].type);
                    hard.update(config_description->list[j].size);
                    hard.update((char *) config_description->list[j].caption);
                }

                if (config_description && config_description->size) {
                    soft.update(item->component->getConfig(), config_description->size);
                }
            }
        }

        ((uint32_t *)packet->part.payload.data)[0] = (uint32_t) hard.get();
        ((uint32_t *)packet->part.payload.data)[1] = (uint32_t) soft.get();
        hydra_fprintln("ENUM RESP");
        return true;
    } else {
        return false;
    }
}
