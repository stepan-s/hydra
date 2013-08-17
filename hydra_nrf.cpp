#include "hydra_nrf.h"
#include <AESLib.h>

static const uint8_t enc_iv[16] = {'H', 'Y', 'D', 'R', 'A', ' ', 'N', 'F', 'R', ' ', 'A', 'E', 'S', ' ', 'I', 'V'};

const char* HydraNrf::name = "NRF24";

const HydraConfigValueDescriptionList HydraNrf::config_value_description_list = {
	4, 22, (HydraConfigValueDescription[]) {
		{2, "ADDR"},
		{3, "NET"},
		{1, "Channel"},
		{16, "EncryptKey"},
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
	uint64_t addr = 0;
	uint64_t bcaddr = 0;
	memcpy(& addr, & this->config.parts.addr, 5);
	memcpy(& bcaddr, & this->config.parts.addr, 5);
	bcaddr |= HYDRA_NRF_BC;
	this->radio->begin();
	this->radio->setChannel(this->config.parts.channel);
	this->radio->setPayloadSize(HYDRA_PACKET_SIZE);
	this->radio->openReadingPipe(1, addr);
	this->radio->openReadingPipe(2, bcaddr);
	this->radio->setRetries(0, 0);
	this->radio->setAutoAck(false);
	this->radio->startListening();
	aes128_enc_single(this->config.parts.enc_key, this->enc_iv);
	hydra_debug("HydraNrf::init end");
}

bool HydraNrf::isPacketAvailable() {
	return this->radio->available();
}

bool HydraNrf::readPacket(HydraPacket* packet) {
	if (this->radio->available()) {
		this->radio->read(packet->data, HYDRA_PACKET_SIZE);
		aes128_cbc_dec(this->config.parts.enc_key, this->enc_iv, packet->data, HYDRA_PACKET_SIZE);
		uint32_t now = this->hydra->getTime();
		if (now && (abs(now - packet->part.timestamp) > 1)) {
			return false;
		}
		return true;
	}
	return false;
}

HydraAddress HydraNrf::getGateway(HydraAddress destionation) {
	if (this->config.parts.addr.part.net == destionation.part.net) {
		if (this->config.parts.addr.part.device == destionation.part.device) {
			destionation.raw = HYDRA_ADDR_LOCAL;
		}
	} else {
		destionation.raw = HYDRA_ADDR_NULL;
	}
	return destionation;
}

bool HydraNrf::sendPacket(const HydraAddress to, const HydraPacket* packet, const bool set_from_addr) {
	hydra_debug_param("HydraNrf::sendPacket to ", to.raw);
	hydra_debug_param("HydraNrf::sendPacket packet to ", packet->part.to_addr.raw);
	this->radio->stopListening();
	uint64_t addr = 0;
	HydraPacket p;
	memcpy(& addr + 1, & this->config.parts.addr + 1, 4);
	memcpy(& p, packet, HYDRA_PACKET_SIZE);
	if (set_from_addr) {
		p.part.from_addr = this->getAddress();
	}
	addr |= to.part.device;
	this->radio->openWritingPipe(addr);
	aes128_cbc_enc(this->config.parts.enc_key, this->enc_iv, p.data, HYDRA_PACKET_SIZE);
	bool result = this->radio->write(p.data, HYDRA_PACKET_SIZE);
	this->radio->startListening();
	return result;
}

HydraAddress HydraNrf::getAddress() {
	return this->config.parts.addr;
}

