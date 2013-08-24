#include "hydra_nrf.h"
#include <AESLib.h>

static const uint8_t enc_iv[16] = {'H', 'Y', 'D', 'R', 'A', ' ', 'N', 'F', 'R', ' ', 'A', 'E', 'S', ' ', 'I', 'V'};

union NrfAddr {
	uint64_t a64;
	uint32_t a32[2];
	uint8_t a8[8];
};

const char* HydraNrf::name = "NRF24";

const HydraConfigValueDescriptionList HydraNrf::config_value_description_list = {
	5, 22 + HYDRA_NRF_ROUTE_COUNT, (HydraConfigValueDescription[]) {
		{2, "ADDR"},
		{3, "NET"},
		{1, "Channel"},
		{16, "EncryptKey"},
		{HYDRA_NRF_ROUTE_COUNT, "RouteToNETS"},
	}
};

HydraNrf::HydraNrf(uint8_t cePin, uint8_t csPin) {
	this->radio = new RF24(cePin, csPin);
	memcpy(this->enc_iv, enc_iv, 16);
}

const char* HydraNrf::getName() {
	return HydraNrf::name;
}

const HydraConfigValueDescriptionList* HydraNrf::getConfigDescription() {
	return & config_value_description_list;
}

uint8_t* HydraNrf::getConfig() {
	return this->config.raw;
}

void HydraNrf::init(Hydra* hydra) {
	hydra_debug("HydraNrf::init begin");
	HydraComponent::init(hydra);
	this->radio->begin();
	//this->radio->setRetries(10, 5);
	this->radio->setPayloadSize(HYDRA_PACKET_SIZE);
	this->radio->setChannel(this->config.parts.channel);

	NrfAddr addr = {0};
	NrfAddr bcaddr = {0};
	memcpy(& addr, & this->config.parts.addr, 5);
	memcpy(& bcaddr, & this->config.parts.addr, 5);
	bcaddr.a8[0] = HYDRA_NRF_BC;

	this->radio->openWritingPipe(bcaddr.a64);

	this->radio->openReadingPipe(1, addr.a64);
	hydra_debug_param("HydraNrf::init listen lo ", addr.a32[0]);
	hydra_debug_param("HydraNrf::init listen hi ", addr.a32[1]);

	this->radio->openReadingPipe(2, bcaddr.a64);
	//this->radio->setAutoAck(2, false);
	hydra_debug_param("HydraNrf::init listen lo ", bcaddr.a32[0]);
	hydra_debug_param("HydraNrf::init listen hi ", bcaddr.a32[1]);

	this->radio->setAutoAck(false);
	this->radio->startListening();
	aes128_enc_single(this->config.parts.enc_key, this->enc_iv);
	hydra_debug("HydraNrf::init end");
}

bool HydraNrf::isPacketAvailable() {
	return this->radio->available();
}

bool HydraNrf::readPacket(HydraPacket* packet) {
	//if (this->radio->available()) {
		hydra_debug("HydraNrf::readPacket");
		this->radio->read(packet->data, HYDRA_PACKET_SIZE);
		aes128_cbc_dec(this->config.parts.enc_key, this->enc_iv, packet->data, HYDRA_PACKET_SIZE);
		hydra_debug_param("HydraNrf::readPacket received from_addr ", packet->part.from_addr.raw);
		hydra_debug_param("HydraNrf::readPacket received to_addr ", packet->part.to_addr.raw);
		uint32_t now = this->hydra->getTime();
		if ((now > 1000000) && (abs(now - packet->part.timestamp) > 5)) {
			hydra_debug_param("HydraNrf::readPacket packet expired ", abs(now - packet->part.timestamp));
			return false;
		}
		return true;
	//}
	//return false;
}

HydraAddress HydraNrf::getGateway(HydraAddress destionation) {
	if (this->config.parts.addr.part.net == destionation.part.net) {
		if (this->config.parts.addr.part.device == destionation.part.device) {
			destionation.raw = HYDRA_ADDR_LOCAL;
		}
	} else {
		int i;
		for(i = 0; i < HYDRA_NRF_ROUTE_COUNT; ++i) {
			if ((this->config.parts.net_routes[i] != 0) && (this->config.parts.net_routes[i] == destionation.part.net)) {
				return destionation;
			}
		}
		destionation.raw = HYDRA_ADDR_NULL;
	}
	return destionation;
}

bool HydraNrf::sendPacket(const HydraAddress to, const HydraPacket* packet, const bool set_from_addr) {
	hydra_debug_param("HydraNrf::sendPacket to ", to.raw);
	hydra_debug_param("HydraNrf::sendPacket packet to ", packet->part.to_addr.raw);
	this->radio->stopListening();

	HydraPacket p;
	memcpy(& p, packet, HYDRA_PACKET_SIZE);
	if (set_from_addr) {
		p.part.from_addr = this->getAddress();
	}

	NrfAddr addr = {0};
	memcpy(& addr, & this->config.parts.addr, 5);
	addr.a8[0] = to.part.device;
	hydra_debug_param("HydraNrf::sendPacket lo ", addr.a32[0]);
	hydra_debug_param("HydraNrf::sendPacket hi ", addr.a32[1]);

	this->radio->openWritingPipe(addr.a64);
	aes128_cbc_enc(this->config.parts.enc_key, this->enc_iv, p.data, HYDRA_PACKET_SIZE);
	bool result = this->radio->write(p.data, HYDRA_PACKET_SIZE);
	this->radio->startListening();
	hydra_debug_param("HydraNrf::sendPacket result ", result);
	return result;
}

HydraAddress HydraNrf::getAddress() {
	return this->config.parts.addr;
}

