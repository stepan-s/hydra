#include "HydraNrf.h"

static const uint8_t _enc_iv[16] = {'H', 'Y', 'D', 'R', 'A', ' ', 'N', 'F', 'R', ' ', 'A', 'E', 'S', ' ', 'I', 'V'};

union NrfAddr {
    uint64_t a64;
    uint32_t a32[2];
    uint8_t a8[8];
};

const char* HydraNrf::name = "NRF24";

const HydraConfigValueDescriptionList HydraNrf::config_value_description_list = {
    6, 24 + HYDRA_NET_ROUTE_COUNT * 2, (HydraConfigValueDescription[]) {
        {HYDRA_CONFIG_VALUE_TYPE_ADDR, 2, "ADDR"},
        {HYDRA_CONFIG_VALUE_TYPE_NET_ROUTE, HYDRA_NET_ROUTE_COUNT * 2, "Routes"},
        {HYDRA_CONFIG_VALUE_TYPE_BINARY, 3, "NET"},
        {HYDRA_CONFIG_VALUE_TYPE_INT, 1, "Channel"},
        {HYDRA_CONFIG_VALUE_TYPE_BINARY, 16, "EncryptKey"},
        {HYDRA_CONFIG_VALUE_TYPE_SET, 2, "RadioOpts"}
    }
};

HydraNrf::HydraNrf(uint8_t cePin, uint8_t csPin) {
    this->radio = new RF24(cePin, csPin);
    memcpy(this->enc_iv, _enc_iv, 16);
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
    this->radio->setAutoAck(false);
    if (this->config.parts.radio_opts.auto_ack) {
        this->radio->enableDynamicPayloads();
        this->radio->setAutoAck(1, true);
    }

    hydra_debug_param("Rtr", this->radio->getRetries());
    hydra_debug_param("Rte", this->radio->getDataRate());
    hydra_debug_param("Crc", this->radio->getCRCLength());
    hydra_debug_param("Pwr", this->radio->getPALevel());
    hydra_debug_param("Tmo", this->radio->getMaxTimeout());

    NrfAddr addr = {0};
    memcpy(& addr, & this->config.parts.addr, 2);
    memcpy(& addr.a8[2], & this->config.parts.net, 3);
    NrfAddr bcaddr = addr;
    bcaddr.a8[0] = HYDRA_NRF_BC;

    this->radio->openWritingPipe(bcaddr.a64);

    this->radio->openReadingPipe(1, addr.a64);
    hydra_debug_param("HydraNrf::init listen lo ", addr.a32[0]);
    hydra_debug_param("HydraNrf::init listen hi ", addr.a32[1]);

    this->radio->openReadingPipe(2, bcaddr.a64);
    hydra_debug_param("HydraNrf::init listen lo ", bcaddr.a32[0]);
    hydra_debug_param("HydraNrf::init listen hi ", bcaddr.a32[1]);

    this->radio->startListening();
    //aes128_enc_single(this->config.parts.enc_key, this->enc_iv);
    uint8_t encryption_enabled = 0;
    for(int i = 0; i < 16; i++) {
        encryption_enabled |= this->config.parts.enc_key[i];
    }
    if (encryption_enabled) {
        this->aes = new AES();
        this->aes->set_key(this->config.parts.enc_key, 16);
        this->aes->encrypt(this->enc_iv, this->enc_iv);
        hydra_debug("HydraNrf::encryption enabled");
    } else {
        this->aes = 0;
        hydra_debug("HydraNrf::encryption disabled");
    }

    // Start auth
    this->auth = false;
    this->auth_timeout.begin(1000);

    hydra_debug("HydraNrf::init end");
}

bool HydraNrf::isPacketAvailable() {
    if (!this->auth) {
        if (this->hydra->isTimeSynced()) {
            // Time in sync -> no auth required
            this->auth = true;
            hydra_fprintln("NRF auth=1 (synced)");
        } else {
            this->auth_timeout.tick();
            if (this->auth_timeout.isEnd()) {
                // Send auth request
                HydraPacket packet;
                memset(&packet, 0, HYDRA_PACKET_SIZE);
                packet.part.from_addr.raw = this->config.parts.addr.raw;
                packet.part.from_service = HYDRA_NET_SERVICE_ID;
                packet.part.to_addr.raw = HYDRA_ADDR_BROADCAST_ALL;
                packet.part.to_service = HYDRA_NET_SERVICE_ID;
                packet.part.timestamp = this->hydra->getTime();
                packet.part.payload.type = HYDRA_NRF_PAYLOAD_TYPE_AUTH_REQUEST;
                this->auth_nonce = this->hydra->rand();
                memcpy(&packet.part.payload.data, &this->auth_nonce, 4);
                this->sendPacket({HYDRA_ADDR_BROADCAST_ALL}, &packet);

                this->auth_timeout.begin(1000);
                hydra_fprintln("NRF auth requested");
            }
        }
    }

    return this->radio->available();
}

bool HydraNrf::readPacket(HydraPacket* packet) {
    hydra_debug("HydraNrf::readPacket");
    uint8_t length = this->radio->getDynamicPayloadSize();
    this->radio->read(packet->data, min(length, HYDRA_PACKET_SIZE));
    if (length == HYDRA_PACKET_SIZE) {
        //aes128_cbc_dec(this->config.parts.enc_key, this->enc_iv, packet->data, HYDRA_PACKET_SIZE);
        if (this->aes) {
            uint8_t iv[16];
            memcpy(iv, this->enc_iv, 16);
            this->aes->cbc_decrypt(packet->data, packet->data, 2, iv);
        }
        hydra_debug_param("HydraNrf::readPacket received from_addr ", packet->part.from_addr.raw);
        hydra_debug_param("HydraNrf::readPacket received to_addr ", packet->part.to_addr.raw);
        uint32_t now = this->hydra->getTime();
        hydra_debug_param("Packet time ", packet->part.timestamp);
        hydra_debug_param("System time ", now);

        if (packet->part.to_service == HYDRA_NET_SERVICE_ID) {
            switch (packet->part.payload.type) {
                case HYDRA_NRF_PAYLOAD_TYPE_AUTH_REQUEST:
                    // Auth request
                    if (this->auth) {
                        hydra_fprint("NRF auth request from ");
                        hydra_hprintln(packet->part.from_addr.raw);
                        // Send auth response
                        HydraPacket packet_response;
                        memset(& packet_response, 0, HYDRA_PACKET_SIZE);
                        packet_response.part.from_addr.raw = this->config.parts.addr.raw;
                        packet_response.part.from_service = HYDRA_NET_SERVICE_ID;
                        packet_response.part.to_addr.raw = packet->part.from_addr.raw;
                        packet_response.part.to_service = HYDRA_NET_SERVICE_ID;
                        packet_response.part.timestamp = this->hydra->getTime();
                        packet_response.part.payload.type = HYDRA_NRF_PAYLOAD_TYPE_AUTH_REPLY;
                        *(uint32_t*)(& packet_response.part.payload.data) = ~*(uint32_t*)(& packet->part.payload.data);
                        *(uint16_t*)(& packet->part.payload.data[4]) = (uint16_t)(this->hydra->getTimeZoneOffset() / 60);
                        this->sendPacket({packet_response.part.to_addr.raw}, & packet_response);
                    } else {
                        hydra_fprint("DROP PACKET cant auth");
                    }
                    break;
                case HYDRA_NRF_PAYLOAD_TYPE_AUTH_REPLY:
                    if (!this->auth) {
                        if (this->auth_nonce != 0) {
                            // Check auth
                            if ((packet->part.to_addr.raw == this->config.parts.addr.raw)
                                && (*(uint32_t *) (&packet->part.payload.data) == ~this->auth_nonce)) {
                                this->auth = true;
                                hydra->setTime(packet->part.timestamp, *(uint16_t *) (&packet->part.payload.data[4]));
                                hydra_fprintln("NRF auth done");
                            } else {
                                hydra_fprintln("NRF auth fail");
                                hydra_debug_param("to ", packet->part.to_addr.raw);
                                hydra_debug_param("from ", packet->part.from_addr.raw);
                                hydra_debug_param("nonce received ", *(uint32_t *) (&packet->part.payload.data));
                                hydra_debug_param("nonce expected ", ~this->auth_nonce);
                                this->auth_nonce = 0;
                            }
                        } else {
                            hydra_fprintln("DROP PACKET not allowed auth");
                        }
                    } else {
                        hydra_fprintln("DROP PACKET auth already");
                    }
                    break;
                default:
                    hydra_fprintln("DROP PACKET unknown");
                    break;
            }
            // packet handled
            return false;
        }

        if (this->auth) {
            if (abs((int32_t) now - (int32_t) packet->part.timestamp) <= 2) {
                //time diff <=2sec -> pass packet
                return true;
            }

            hydra_fprintln("DROP PACKET invalid time/encryption");
            hydra_debug_param("HydraNrf::readPacket packet expired ", abs(now - packet->part.timestamp));

            if (!this->hydra->isTimeSynced()) {
                // Time out of sync -> auth require
                this->auth = false;
                hydra_fprintln("NRF auth=0 (out of sync)");
            }
        } else {
            // Auth required
            hydra_fprintln("DROP PACKET nrf auth required");
        }
    } else {
        hydra_fprintln("DROP PACKET invalid size");
    }
    return false;
}

bool HydraNrf::sendPacket(const HydraAddress to, const HydraPacket* packet) {
    hydra_debug_param("HydraNrf::sendPacket to ", to.raw);
    hydra_debug_param("HydraNrf::sendPacket packet to ", packet->part.to_addr.raw);

    HydraPacket p;
    memcpy(& p, packet, HYDRA_PACKET_SIZE);

    NrfAddr addr = {0};
    memcpy(& addr, & this->config.parts.addr, 2);
    memcpy(& addr.a8[2], & this->config.parts.net, 3);
    addr.a8[0] = to.part.device;
    hydra_debug_param("HydraNrf::sendPacket lo ", addr.a32[0]);
    hydra_debug_param("HydraNrf::sendPacket hi ", addr.a32[1]);

    //aes128_cbc_enc(this->config.parts.enc_key, this->enc_iv, p.data, HYDRA_PACKET_SIZE);
    if (this->aes) {
        uint8_t iv[16];
        memcpy(iv, this->enc_iv, 16);
        this->aes->cbc_encrypt(p.data, p.data, 2, iv);
    }

    this->radio->stopListening();
    this->radio->openWritingPipe(addr.a64);
    bool result = this->radio->write(p.data, HYDRA_PACKET_SIZE, hydra_is_addr_to_net(to));
    this->radio->startListening();

    hydra_debug_param("HydraNrf::sendPacket result ", result);
    return result;
}
