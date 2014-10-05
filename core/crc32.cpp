#include <avr/pgmspace.h>
#include "crc32.h"

static const PROGMEM uint32_t crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

Crc32::Crc32() {
	crc = 0xFFFFFFFF;
}

void Crc32::update(const uint8_t data) {
	uint8_t tbl_idx;
    tbl_idx = crc ^ (data >> (0 * 4));
    crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
    tbl_idx = crc ^ (data >> (1 * 4));
    crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
}

void Crc32::update(const uint16_t data) {
	update((uint8_t *)&data, 2);
}

void Crc32::update(const uint32_t data) {
	update((uint8_t *)&data, 4);
}

void Crc32::update(const uint64_t data) {
	update((uint8_t *)&data, 8);
}

void Crc32::update(const char *s) {
	if (s) {
		while (*s) {
			update((uint8_t)*s++);
		}
	}
}

void Crc32::update(const uint8_t *buf, const int length) {
	int pos = 0;
	while (pos < length) {
		update(buf[pos++]);
	}
}

unsigned long Crc32::get() {
	return ~crc;
}
