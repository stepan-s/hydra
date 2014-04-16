#include "hydra_nrf.h"
#include <AESLib.h>
#include "hydra_core.h"

static const uint8_t enc_iv[16] = {'H', 'Y', 'D', 'R', 'A', ' ', 'N', 'F', 'R', ' ', 'A', 'E', 'S', ' ', 'I', 'V'};

union NrfAddr {
	uint64_t a64;
	uint32_t a32[2];
	uint8_t a8[8];
};

const char* HydraNrf::name = "NRF24";

const HydraConfigValueDescriptionList HydraNrf::config_value_description_list = {
	6, 24 + HYDRA_NRF_ROUTE_COUNT, (HydraConfigValueDescription[]) {
		{2, "ADDR"},
		{3, "NET"},
		{1, "Channel"},
		{16, "EncryptKey"},
		{HYDRA_NRF_ROUTE_COUNT, "RouteToNETS"},
		{2, "RadioOpts"}
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
	this->radio->setRetries(this->config.parts.radio_opts.retries_delay, this->config.parts.radio_opts.retries_count);
	this->radio->setDataRate((rf24_datarate_e)(this->config.parts.radio_opts.speed));
	this->radio->setPALevel((rf24_pa_dbm_e)this->config.parts.radio_opts.power);
	this->radio->setCRCLength((rf24_crclength_e)this->config.parts.radio_opts.crc);
	this->radio->setPayloadSize(HYDRA_PACKET_SIZE);
	this->radio->setChannel(this->config.parts.channel);

	hydra_debug_param("Rtr", this->radio->getRetries());
	hydra_debug_param("Rte", this->radio->getDataRate());
	hydra_debug_param("Crc", this->radio->getCRCLength());
	hydra_debug_param("Pwr", this->radio->getPALevel());
	hydra_debug_param("Tmo", this->radio->getMaxTimeout());

	NrfAddr addr = {0};
	NrfAddr bcaddr = {0};
	memcpy(& addr, & this->config.parts.addr, 5);
	memcpy(& bcaddr, & this->config.parts.addr, 5);
	bcaddr.a8[0] = HYDRA_NRF_BC;

	this->radio->openWritingPipe(bcaddr.a64);

	this->radio->openReadingPipe(1, addr.a64);
	this->radio->setAutoAck(1, true);
	hydra_debug_param("HydraNrf::init listen lo ", addr.a32[0]);
	hydra_debug_param("HydraNrf::init listen hi ", addr.a32[1]);

	this->radio->openReadingPipe(2, bcaddr.a64);
	this->radio->setAutoAck(2, false);
	hydra_debug_param("HydraNrf::init listen lo ", bcaddr.a32[0]);
	hydra_debug_param("HydraNrf::init listen hi ", bcaddr.a32[1]);

	if (!this->config.parts.radio_opts.auto_ack) {
		this->radio->setAutoAck(false);
	}
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
		if ((abs(now - packet->part.timestamp) > 2) and !((packet->part.to_service == HYDRA_SERVICE_CORE) and (packet->part.payload.type == HYDRA_PAYLOAD_CORE_TYPE_SET_TIME))) {
			hydra_debug_param("HydraNrf::readPacket packet expired ", abs(now - packet->part.timestamp));
			return false;
		}
		return true;
	//}
	//return false;
}

HydraAddress HydraNrf::getGateway(const HydraAddress destionation) {
	HydraAddress gateway = destionation;
	if (this->config.parts.addr.part.net == destionation.part.net) {
		if (this->config.parts.addr.part.device == destionation.part.device) {
			gateway.raw = HYDRA_ADDR_LOCAL;
		}
	} else {
		int i;
		for(i = 0; i < HYDRA_NRF_ROUTE_COUNT; ++i) {
			if ((this->config.parts.net_routes[i] != 0) && (this->config.parts.net_routes[i] == destionation.part.net)) {
				return destionation;
			}
		}
		gateway.raw = HYDRA_ADDR_NULL;
	}
	return gateway;
}

bool HydraNrf::sendPacket(const HydraAddress to, const HydraPacket* packet, const bool set_from_addr) {
	hydra_debug_param("HydraNrf::sendPacket to ", to.raw);
	hydra_debug_param("HydraNrf::sendPacket packet to ", packet->part.to_addr.raw);

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

	aes128_cbc_enc(this->config.parts.enc_key, this->enc_iv, p.data, HYDRA_PACKET_SIZE);

	this->radio->stopListening();
	this->radio->openWritingPipe(addr.a64);
	bool result = this->radio->write(p.data, HYDRA_PACKET_SIZE, hydra_is_addr_to_net(to));
	this->radio->startListening();

	hydra_debug_param("HydraNrf::sendPacket result ", result);
	return result;
}

HydraAddress HydraNrf::getAddress() {
	return this->config.parts.addr;
}
