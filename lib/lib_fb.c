#include <stdbool.h>
#include "lib_fb.h"

#define PIN_CS      (1 << 6)

#define SPI_START   0x70
#define SPI_RD      0x01
#define SPI_WR      0x00
#define SPI_DATA    0x02
#define SPI_INDEX   0x00

#define SSP_TFE     0x01
#define SSP_RNE     0x04
#define SSP_BSY     0x10

static void gpio_set_bit(volatile LPC_GPIO_TypeDef *gpio, unsigned int bit, bool set)
{
    volatile uint32_t *r = set
        ? &gpio->FIOSET
        : &gpio->FIOCLR;
    r[0] |= (1 << bit);
}

static uint8_t spi_write(uint8_t value)
{
    LPC_SSP1->DR = value;
    while (!(LPC_SSP1->SR & SSP_RNE));
    return (uint8_t)(LPC_SSP1->DR);
}

static void write_command(uint8_t cmd)
{
    gpio_set_bit(LPC_GPIO0, PIN_CS, false);

    spi_send(SPI_START | SPI_WR | SPI_INDEX);
    spi_send(0x00);
    spi_send(cmd);

    gpio_set_bit(LPC_GPIO0, PIN_CS, true);
}

static void write_register(uint8_t reg, uint16_t value)
{
    gpio_set_bit(LPC_GPIO0, PIN_CS, false);
    spi_send(SPI_START | SPI_WR | SPI_INDEX);
    spi_send(0x00);
    spi_send(reg);
    gpio_set_bit(LPC_GPIO0, PIN_CS, true);

    gpio_set_bit(LPC_GPIO0, PIN_CS, false);
    spi_send(SPI_START | SPI_WR | SPI_DATA);
    spi_send((value >> 8));
    spi_send((value & 0xFF));
    gpio_set_bit(LPC_GPIO0, PIN_CS, true);
}

void fb_init(void)
{
    /* IDK yet, will reverse-engineer this bit */
}

void fb_clear(uint16_t color, const struct fb_rect *rect)
{
    unsigned long i, j;
    fb_batch_begin(rect);
    for(i = 0; i < rect->width; i++)
        for(j = 0; j < rect->height; j++)
            fb_batch_pixel(color, i + rect->xpos, j + rect->ypos);
    fb_batch_end();
}

void fb_fclear(const struct fb_font *font, uint16_t color, const struct fb_rect *rect)
{
    struct fb_rect frect = { 0 };
    frect.xpos = rect->xpos * font->width;
    frect.ypos = rect->ypos * font->height;
    frect.width = rect->width * font->width;
    frect.height = rect->height * font->height;
    fb_clear(color, &frect);
}

void fb_fputchar(const struct fb_font *font, uint16_t fg, uint16_t bg, unsigned long xpos, unsigned long ypos, char c)
{
    /* wait for this */
}

void fb_fputs(const struct fb_font *font, uint16_t fg, uint16_t bg, unsigned long xpos, unsigned long ypos, const char *s)
{
    /* wait for this */
}

void fb_batch_begin(const struct fb_rect *rect)
{

}

void fb_batch_pixel(uint16_t color, unsigned long xpos, unsigned long ypos)
{

}

void fb_batch_fchar(const struct fb_font *font, uint16_t fg, uint16_t bg, unsigned long xpos, unsigned long ypos, char c)
{

}

void fb_batch_rect(uint16_t color, const struct fb_rect *rect)
{

}

void fb_batch_end(void)
{

}
