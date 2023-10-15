#ifndef A1586A61_CB02_42BE_9F21_90C4BF8DCEC7
#define A1586A61_CB02_42BE_9F21_90C4BF8DCEC7
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define LCD_WIDTH   (320)
#define LCD_HEIGHT  (240)
#define LCD_LENGTH  (LCD_WIDTH * LCD_HEIGHT)

struct bfont {
    size_t char_width;
    size_t char_height;
    size_t ascii_offset;
    const void *data;
};

void lcd_init(void);
void lcd_clear(uint16_t color);
void lcd_rect(uint16_t color, size_t x, size_t y, size_t w, size_t h);
void lcd_bitmap(size_t x, size_t y, size_t w, size_t h, const uint16_t *bitmap);
void lcd_bchar(const struct bfont *fnt, uint16_t fg, uint16_t bg, size_t cx, size_t cy, char ch);
void lcd_bcline(const struct bfont *fnt, uint16_t bg, size_t cy);

void lcd_bputs(const struct bfont *fnt, uint16_t fg, uint16_t bg, size_t cx, size_t cy, const char *s);
void lcd_bprintf(const struct bfont *fnt, uint16_t fg, uint16_t bg, size_t cx, size_t cy, const char *fmt, ...);
void lcd_bvprintf(const struct bfont *fnt, uint16_t fg, uint16_t bg, size_t cx, size_t cy, const char *fmt, va_list ap);

#endif /* A1586A61_CB02_42BE_9F21_90C4BF8DCEC7 */
