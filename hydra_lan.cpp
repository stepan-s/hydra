#include "hydra_lan.h"

const char* HydraLan::name = "LAN";

const HydraConfigValueDescriptionList HydraLan::config_value_description_list = {
	6, 20, (HydraConfigValueDescription[]) {
		{2, "ADDR"},
		{6, "MAC"},
		{4, "LocalIP"},
		{2, "LocalPORT"},
		{4, "GatewayIP"},
		{2, "GatewayPORT"}
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
	this->gateway_ip = IPAddress(this->config.parts.gateway_ip);
	Ethernet.begin(this->config.parts.mac, IPAddress(this->config.parts.local_ip), this->gateway_ip, this->gateway_ip);
	this->udp.begin(this->config.parts.local_port);
	hydra_debug_param("HydraLan::init IP ", IPAddress(this->config.parts.local_ip));
	hydra_debug_param("HydraLan::init PORT ", this->config.parts.local_port);
	hydra_debug("HydraLan::init end");
}

bool HydraLan::isPacketAvailable() {
	this->udp.parsePacket();
	return this->udp.available();
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
			//this->udp.flush();
			return readed == HYDRA_PACKET_SIZE;
		}
	}
	return false;
}

HydraAddress HydraLan::getGateway(const HydraAddress destionation) {
	HydraAddress gateway = destionation;
	if (this->config.parts.addr.part.net == destionation.part.net) {
		if (this->config.parts.addr.part.device == destionation.part.device) {
			gateway.raw = HYDRA_ADDR_LOCAL;
		}
	} else {
		gateway.raw = HYDRA_ADDR_NULL;
	}
	return gateway;
}

bool HydraLan::sendPacket(const HydraAddress to, const HydraPacket* packet, const bool set_from_addr) {
	HydraPacket p;
	memcpy(& p, packet, HYDRA_PACKET_SIZE);
	if (set_from_addr) {
		p.part.from_addr = this->getAddress();
	}
	hydra_debug_param("HydraLan::sendPacket to ", to.raw);
	hydra_debug_param("HydraLan::sendPacket ip ", this->gateway_ip);
	hydra_debug_param("HydraLan::sendPacket packet to ", p.part.to_addr.raw);
    this->udp.beginPacket(this->gateway_ip, this->config.parts.gateway_port);
    this->udp.write(p.data, HYDRA_PACKET_SIZE);
    this->udp.endPacket();
	return true;
}

HydraAddress HydraLan::getAddress() {
	return this->config.parts.addr;
}
