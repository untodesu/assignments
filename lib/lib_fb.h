#ifndef C86A2504_85A6_49CE_8374_076CCA1FE423
#define C86A2504_85A6_49CE_8374_076CCA1FE423
#include <lpc17xx.h>

struct fb_font {
    unsigned long width;
    unsigned long height;
    const unsigned char *data;
};

struct fb_rect {
    unsigned long xpos;
    unsigned long ypos;
    unsigned long width;
    unsigned long height;
};

void fb_init(void);
void fb_clear(uint16_t color, const struct fb_rect *rect);
void fb_fclear(const struct fb_font *font, uint16_t color, const struct fb_rect *rect);
void fb_fputchar(const struct fb_font *font, uint16_t fg, uint16_t bg, unsigned long xpos, unsigned long ypos, char c);
void fb_fputs(const struct fb_font *font, uint16_t fg, uint16_t bg, unsigned long xpos, unsigned long ypos, const char *s);

void fb_batch_begin(const struct fb_rect *rect);
void fb_batch_pixel(uint16_t color, unsigned long xpos, unsigned long ypos);
void fb_batch_fchar(const struct fb_font *font, uint16_t fg, uint16_t bg, unsigned long xpos, unsigned long ypos, char c);
void fb_batch_rect(uint16_t color, const struct fb_rect *rect);
void fb_batch_end(void);

#endif /* C86A2504_85A6_49CE_8374_076CCA1FE423 */
