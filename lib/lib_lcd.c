#include <stdbool.h>
#include "lib_lcd.h"

#define LCD_PIN_CS  0x06
#define LCD_PIN_CLK 0x07
#define LCD_PIN_DAT 0x09

#define SPI_START   0x70
#define SPI_READ    0x01
#define SPI_WRITE   0x00
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
    r[0] |= (1U << bit);
}

static uint8_t spi_write(uint8_t value)
{
    LPC_SSP1->DR = (uint32_t)value;
    while(!(LPC_SSP1->SR & SSP_RNE));
    return (uint8_t)LPC_SSP1->DR;
}

static void write_cmd(uint8_t value)
{
    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, false);
    spi_write(SPI_START | SPI_WRITE | SPI_INDEX);
    spi_write(0x00);
    spi_write(value);
    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, true);
}

static void write_dat(uint16_t value)
{
    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, false);
    spi_write(SPI_START | SPI_WRITE | SPI_DATA);
    spi_write((value & 0xFF00) >> 8);
    spi_write((value & 0x00FF));
    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, true);
}

static void write_dat_only(uint16_t value)
{
    spi_write((value & 0xFF00) >> 8);
    spi_write((value & 0x00FF));
}

static void write_reg(uint8_t reg, uint16_t value)
{
    write_cmd(reg);
    write_dat(value);
}

static void write_buf(const uint16_t *buffer, size_t size)
{
    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, false);
    spi_write(SPI_START | SPI_WRITE | SPI_DATA);
    while(size--)
        write_dat_only(*buffer++);
    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, true);
}

/* We use a backbuffer to provide
 * a simple framebuffer-like object
 * for possibly any code to use. We
 * submit backbuffer data infrequently. */
uint16_t lcd_backbuffer[LCD_LENGTH] = { 0 };

void lcd_init(void)
{
    /* IDK yet, probably a copy of GLCD_Init code here */
}

void lcd_submit(void)
{
    /* Set draw region */
    write_reg(0x50, (uint16_t)(0));
    write_reg(0x51, (uint16_t)(LCD_HEIGHT - 1));
    write_reg(0x52, (uint16_t)(0));
    write_reg(0x53, (uint16_t)(LCD_WIDTH - 1));
    write_reg(0x20, (uint16_t)(0));
    write_reg(0x21, (uint16_t)(0));

    /* Dump the backbuffer */
    write_cmd(0x22);
    write_buf(lcd_backbuffer, LCD_LENGTH);
}

void lcd_clear(uint16_t color)
{
    size_t i;
    for(i = 0; i < LCD_LENGTH; i++)
        lcd_backbuffer[i] = color;
}

void lcd_rect(uint16_t color, size_t x, size_t y, size_t width, size_t height, bool fill)
{
    size_t i;
    size_t begin, end;

    if(fill) {
        begin = LCD_PIX(x, y);
        end = LCD_PIX(x + width, y + height);
        for(i = begin; i < end; i++)
            lcd_backbuffer[i] = color;
        return;
    }

    for(i = 0; i < width; i++) {
        lcd_backbuffer[LCD_PIX(x + i, y)] = color;
        lcd_backbuffer[LCD_PIX(x + i, y + height)] = color;
    }
        
    for(i = 0; i < height; i++) {
        lcd_backbuffer[LCD_PIX(x, y + i)] = color;
        lcd_backbuffer[LCD_PIX(x + width, y + i)] = color;
    }
}

void lcd_bfont_char(const struct bfont *font, uint16_t fg, uint16_t bg, size_t cx, size_t cy, char c)
{
    size_t i, j, w, pix;
    const uint8_t *dat = &font->data[c * font->char_height];

    cx *= font->char_width;
    cy *= font->char_height;

    w = 1;
    for(j = 0; j < font->char_height; j++) {
        for(i = 0; i < font->char_width; i++) {
            pix = LCD_PIX(cx + i, cy + j);
            if(dat[0] & (1 << i))
                lcd_backbuffer[pix] = fg;
            else
                lcd_backbuffer[pix] = bg;
            if((i + 1) / (w * 8)) {
                dat++;
                w++;
            }
        }
    }
}
