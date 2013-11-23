#include "lcd.h"
#include <Arduino.h>

//bit7-3 - 0-12 - byte in lcd buffer
//bit2-0 - 0-7 - bite in byte of lcd buffer
const uint8_t lcd_pixel_map[4][3][5] = {
  {
    {
      0 * 8 + 2,
      0 * 8 + 3,
      0 * 8 + 4,
      0 * 8 + 5,
      0 * 8 + 6,
    },
    {
      0 * 8 + 1,
      3 * 8 + 1,
      6 * 8 + 1,
      9 * 8 + 1,
      2 * 8 + 6,
    },
    {
      0 * 8 + 0,
      3 * 8 + 0,
      6 * 8 + 0,
      9 * 8 + 0,
      11 * 8 + 6,
    },
  },
  {
    {
      9 * 8 + 2,
      9 * 8 + 3,
      9 * 8 + 4,
      9 * 8 + 5,
      9 * 8 + 6,
    },
    {
      6 * 8 + 2,
      6 * 8 + 3,
      6 * 8 + 4,
      6 * 8 + 5,
      6 * 8 + 6,
    },
    {
      3 * 8 + 2,
      3 * 8 + 3,
      3 * 8 + 4,
      3 * 8 + 5,
      3 * 8 + 6,
    },
  },
  {
    {
      1 * 8 + 0,
      1 * 8 + 1,
      1 * 8 + 2,
      1 * 8 + 3,
      1 * 8 + 4,
    },
    {
      2 * 8 + 4,
      10 * 8 + 7,
      7 * 8 + 7,
      1 * 8 + 7,
      4 * 8 + 7,
    },
    {
      11 * 8 + 4,
      11 * 8 + 3,
      8 * 8 + 3,
      2 * 8 + 3,
      5 * 8 + 3,
    },
  },
  {
    {
      10 * 8 + 0,
      10 * 8 + 1,
      10 * 8 + 2,
      10 * 8 + 3,
      10 * 8 + 4,
    },
    {
      7 * 8 + 0,
      7 * 8 + 1,
      7 * 8 + 2,
      7 * 8 + 3,
      7 * 8 + 4,
    },
    {
      4 * 8 + 0,
      4 * 8 + 1,
      4 * 8 + 2,
      4 * 8 + 3,
      4 * 8 + 4,
    },
  },
};

const uint8_t lcd_pixel_map_2[2] = {
	5 * 8 + 6,
	8 * 8 + 6,
};

const uint8_t lcd_bitmap_font[10][3] = {
  {0b11111000,
   0b10001000,
   0b11111000},
  {0b00001000,
   0b11111000,
   0b01001000},
  {0b11101000,
   0b10101000,
   0b10111000},
  {0b11111000,
   0b10101000,
   0b10001000},
  {0b11111000,
   0b00100000,
   0b11100000},
  {0b10111000,
   0b10101000,
   0b11101000},
  {0b10111000,
   0b10101000,
   0b11111000},
  {0b11111000,
   0b10000000,
   0b10000000},
  {0b11111000,
   0b10101000,
   0b11111000},
  {0b11111000,
   0b10101000,
   0b11101000},
};

uint8_t lcd_bitmap[4][3] = {
	{0b00000000, 0b00000000, 0b00000000},
	{0b00000000, 0b00000000, 0b00000000},
	{0b00000000, 0b00000000, 0b00000000},
	{0b00000000, 0b00000000, 0b00000000}
};
uint8_t lcd_phase = 0;

void lcd_render_symbol(int pos, int symbol) {
	uint8_t font_col, font_row;
	for (font_col = 0; font_col < 3; ++font_col) {
		uint8_t pix_col = lcd_bitmap_font[symbol][font_col];
		for (font_row = 0; font_row < 5; ++font_row) {
			uint8_t pix_map = lcd_pixel_map[pos][font_col][font_row];
			uint8_t buffer_index = (pix_map & 0b11111000) >> 3;
			uint8_t buffer_shift = (pix_map & 0b00000111);
			uint8_t buffer_value = 1 << buffer_shift;
			if (pix_col & 0x80) {
				((uint8_t *) lcd_bitmap)[buffer_index] |= buffer_value;
			} else {
				((uint8_t *) lcd_bitmap)[buffer_index] &= ~buffer_value;
			}
			pix_col <<= 1;
		}
	}
}

void lcd_render_pixel(int pixel, bool active) {
	uint8_t pix_map = lcd_pixel_map_2[pixel];
	uint8_t buffer_index = (pix_map & 0b11111000) >> 3;
	uint8_t buffer_shift = (pix_map & 0b00000111);
	uint8_t buffer_value = 1 << buffer_shift;
	if (active) {
		((uint8_t *) lcd_bitmap)[buffer_index] |= buffer_value;
	} else {
		((uint8_t *) lcd_bitmap)[buffer_index] &= ~buffer_value;
	}
}

void lcd_rest() {
#ifdef __AVR_ATmega2560__
	PORTA = 0;
	DDRA = 0xFF;
	PORTC = 0;
	DDRC = 0xFF;
	PORTL = 0;
	DDRL = 0xFF;
	PORTD = PORTD & 0x7F;
	DDRD = DDRD | 0x80;
#endif
}

void lcd_set() {
#ifdef __AVR_ATmega2560__
	//lcd phase has two half phases, get true phase
	uint8_t phase = lcd_phase >> 1;
	//get phase bitmap bank
	uint8_t* bitmap = lcd_bitmap[phase];
	//get ports state
	uint8_t pa = bitmap[0];
	uint8_t pc = bitmap[1];
	uint8_t pl = bitmap[2];
	uint8_t pd = PORTD & 0x7F; //only for one backplane pin
	if (lcd_phase & 1) {
		//if second half phase invert ports state
		pa ^= 0xFF;
		pc ^= 0xFF;
		pl ^= 0xFF;
	}
	//reset backplane bits
	pl &= 0xF8;

	uint8_t backplane = 1 << phase;
	if (lcd_phase & 1) {
		//raise backpane if second half phase
		if (phase == 3) {
			pd |= 0x80;
		} else {
			pl |= backplane;
		}
	}

	uint8_t da, dc, dl, dd;

	da = 0x0;
	dc = 0x0;
	dl = 0x0 & 0xF8;
	dd = DDRD & 0x7F;

	if (phase == 3) {
		dd |= 0x80;
	} else {
		dl |= backplane;
	}

	//set port A
	PORTA = pa;
	DDRA = da;
	//set port C
	PORTC = pc;
	DDRC = dc;
	//set port L with 3 backplanes
	PORTL = pl;
	DDRL = dl;
	//set port D 1 backplane
	PORTD = pd;
	DDRD = dd;

	//next lcd phase
	if (++lcd_phase > 7) {
		lcd_phase = 0;
	}
#endif
}

uint8_t lcd_state = 0;
#ifdef __AVR_ATmega2560__
ISR(TIMER5_COMPA_vect) {
	if (lcd_state == 0) {
		lcd_rest();
	} else if (lcd_state == 3) {
		lcd_set();
	}
	++lcd_state;
	if (lcd_state >= 10) {
		lcd_state = 0;
	}
	TCNT5 = 0;
}
#endif

void lcd_init() {
#ifdef __AVR_ATmega2560__
	cli();
	TCCR5A = 0;
	TCCR5B = 1 << CS10; // Set CS10 bit so timer runs at clock speed
	TIMSK5 = 1 << OCIE1A; // compare enable
	OCR5A = 3200; // compare value
	sei();
#endif
}
