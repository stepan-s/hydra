#ifndef _LCD_
#define _LCD_ true

#define LCD_PIXEL_WATCH_DOT_HI 0
#define LCD_PIXEL_WATCH_DOT_LO 1

void lcd_init();
void lcd_render_symbol(int pos, int symbol);
void lcd_render_pixel(int pixel, bool active);

#endif
