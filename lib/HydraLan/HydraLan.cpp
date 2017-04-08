#include "HydraLan.h"

const char* HydraLan::name = "LAN";

const HydraConfigValueDescriptionList HydraLan::config_value_description_list = {
    5, 14 + HYDRA_NET_ROUTE_COUNT * 2, (HydraConfigValueDescription[]) {
        {HYDRA_CONFIG_VALUE_TYPE_ADDR, 2, "ADDR"},
        {HYDRA_CONFIG_VALUE_TYPE_NET_ROUTE, HYDRA_NET_ROUTE_COUNT * 2, "Routes"},
        {HYDRA_CONFIG_VALUE_TYPE_MAC, 6, "MAC"},
        {HYDRA_CONFIG_VALUE_TYPE_IP, 4, "IP"},
        {HYDRA_CONFIG_VALUE_TYPE_INT, 2, "PORT"}
    }
};

HydraLan::HydraLan() {
}

const char* HydraLan::getName() {
    return HydraLan::name;
}

const HydraConfigValueDescriptionList* HydraLan::getConfigDescription() {
    return & config_value_description_list;
}

uint8_t* HydraLan::getConfig() {
    return this->config.raw;
}

void HydraLan::init(Hydra* hydra) {
    hydra_debug("HydraLan::init begin");
    HydraComponent::init(hydra);
    Ethernet.begin(this->config.parts.mac, IPAddress(this->config.parts.ip));
    this->udp.begin(this->config.parts.port);
    hydra_debug_param("HydraLan::init IP ", IPAddress(this->config.parts.ip));
    hydra_debug_param("HydraLan::init PORT ", this->config.parts.port);
    hydra_debug("HydraLan::init end");
}

bool HydraLan::isPacketAvailable() {
    return this->udp.parsePacket();
}

bool HydraLan::readPacket(HydraPacket* packet) {
    hydra_debug("HydraLan::readPacket");
    int packetSize = this->udp.available();
    if (packetSize != 0) {
        hydra_debug_param("HydraLan::readPacket received from ", this->udp.remoteIP());
        hydra_debug_param("HydraLan::readPacket received size ", packetSize);
        if (HYDRA_PACKET_SIZE == packetSize) {
            int readed = this->udp.read(packet->data, HYDRA_PACKET_SIZE);
            hydra_debug_param("HydraLan::readPacket read ", readed);
            hydra_debug_param("HydraLan::readPacket time ", packet->part.timestamp);
            return readed == HYDRA_PACKET_SIZE;
        }
    }
    return false;
}

bool HydraLan::sendPacket(const HydraAddress to, const HydraPacket* packet) {
    IPAddress to_ip = this->config.parts.ip;
    to_ip[3] = to.part.device;
    hydra_debug_param("HydraLan::sendPacket to ", to.raw);
    hydra_debug_param("HydraLan::sendPacket ip ", to_ip);
    hydra_debug_param("HydraLan::sendPacket packet to ", packet->part.to_addr.raw);
    this->udp.beginPacket(to_ip, this->config.parts.port);
    this->udp.write(packet->data, HYDRA_PACKET_SIZE);
    this->udp.endPacket();
    return true;
}
