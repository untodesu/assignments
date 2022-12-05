#include <lpc17xx.h>
#include <stdio.h>
#include <string.h>
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

static void gpio_set_bit(volatile LPC_GPIO_TypeDef *gpio, unsigned int bit, int set)
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
    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, 0);
    spi_write(SPI_START | SPI_WRITE | SPI_INDEX);
    spi_write(0x00);
    spi_write(value);
    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, 1);
}

static void write_dat(uint16_t value)
{
    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, 0);
    spi_write(SPI_START | SPI_WRITE | SPI_DATA);
    spi_write((value & 0xFF00) >> 8);
    spi_write((value & 0x00FF));
    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, 1);
}

static void write_reg(uint8_t reg, uint16_t value)
{
    write_cmd(reg);
    write_dat(value);
}

static void write_batch_begin(void)
{
    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, 0);
    spi_write(SPI_START | SPI_WRITE | SPI_DATA);
}

static void write_batch_end(void)
{
    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, 1);
}

static void write_batch_buffer(const uint16_t *buffer, size_t n)
{
    while(n--) {
        spi_write((*buffer & 0xFF00) >> 8);
        spi_write((*buffer & 0x00FF));
        buffer++;
    }
}

static void write_batch_value(uint16_t value, size_t n)
{
    while(n--) {
        spi_write((value & 0xFF00) >> 8);
        spi_write((value & 0x00FF));
    }
}

static uint16_t read_dat(void)
{
    uint16_t value = 0;

    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, 0);
    spi_write(SPI_START | SPI_READ | SPI_DATA);
    spi_write(0x00);

    value |= (spi_write(0x00) << 8) & 0xFF00;
    value |= (spi_write(0x00) << 0) & 0x00FF;

    gpio_set_bit(LPC_GPIO0, LCD_PIN_CS, 1);

    return value;
}

static uint16_t read_reg(uint8_t reg)
{
    write_cmd(reg);
    return read_dat();
}

#define DELAY_2N 18
static void delay(int cnt)
{
    cnt <<= DELAY_2N;
    while (cnt--);
}

void lcd_init(void)
{
    unsigned short driver_code;

    /* enable clock for SSP1 */
    LPC_SC->PCONP |= 0x00000400;
    LPC_SC->PCLKSEL0 |= 0x00200000;

    /* configure control pins */
    LPC_PINCON->PINSEL9 &= 0xF0FFFFFF;
    LPC_GPIO4->FIODIR |= 0x30000000;
    LPC_GPIO4->FIOSET = 0x20000000;

    /* SSEL1 is GPIO output set to high */
    LPC_GPIO0->FIODIR |= 0x00000040;
    LPC_GPIO0->FIOSET = 0x00000040;
    LPC_PINCON->PINSEL0 &= 0xFFF03FFF;
    LPC_PINCON->PINSEL0 |= 0x000A8000;

    /* enable SPI in master mode, cpol=1, cpha=1 */
    /* max 25 Mbit used for data transfer at 100 MHz */
    LPC_SSP1->CR0 = 0xC7;
    LPC_SSP1->CPSR = 0x02;
    LPC_SSP1->CR1 = 0x02;

    /* delay 50 ms */
    delay(5);

    driver_code = read_reg(0x00);

    /* start initial sequence */
    write_reg(0x01, 0x0100);    /* Set SS bit                           */
    write_reg(0x02, 0x0700);    /* Set 1 line inversion                 */
    write_reg(0x04, 0x0000);    /* Resize register                      */
    write_reg(0x08, 0x0207);    /* 2 lines front, 7 back porch          */
    write_reg(0x09, 0x0000);    /* Set non-disp area refresh cyc ISC    */
    write_reg(0x0A, 0x0000);    /* FMARK function                       */
    write_reg(0x0C, 0x0000);    /* RGB interface setting                */
    write_reg(0x0D, 0x0000);    /* Frame marker Position                */
    write_reg(0x0F, 0x0000);    /* RGB interface polarity               */

    /* Power-on sequence */
    write_reg(0x10, 0x0000);    /* Reset Power Control 1                */
    write_reg(0x11, 0x0000);    /* Reset Power Control 2                */
    write_reg(0x12, 0x0000);    /* Reset Power Control 3                */
    write_reg(0x13, 0x0000);    /* Reset Power Control 4                */
    delay(20);                  /* Discharge cap power voltage (200ms)  */

    write_reg(0x10, 0x12B0);    /* SAP, BT[3:0], AP, DSTB, SLP, STB     */
    write_reg(0x11, 0x0007);    /* DC1[2:0], DC0[2:0], VC[2:0]          */
    delay(5);                   /* Delay 50 ms                          */
  
    write_reg(0x12, 0x01BD);    /* VREG1OUT voltage                     */
    delay(5);                   /* Delay 50 ms                          */

    write_reg(0x13, 0x1400);    /* VDV[4:0] for VCOM amplitude          */
    write_reg(0x29, 0x000E);    /* VCM[4:0] for VCOMH                   */
    delay(5);                   /* Delay 50 ms                          */

    write_reg(0x20, 0x0000);    /* GRAM horizontal Address              */
    write_reg(0x21, 0x0000);    /* GRAM Vertical Address                */

    /* Adjust the Gamma Curve */
    
    if (driver_code == 0x5408) {
        write_reg(0x30, 0x0B0D);
        write_reg(0x31, 0x1923);
        write_reg(0x32, 0x1C26);
        write_reg(0x33, 0x261C);
        write_reg(0x34, 0x2419);
        write_reg(0x35, 0x0D0B);
        write_reg(0x36, 0x1006);
        write_reg(0x37, 0x0610);
        write_reg(0x38, 0x0706);
        write_reg(0x39, 0x0304);
        write_reg(0x3A, 0x0E05);
        write_reg(0x3B, 0x0E01);
        write_reg(0x3C, 0x010E);
        write_reg(0x3D, 0x050E);
        write_reg(0x3E, 0x0403);
        write_reg(0x3F, 0x0607);
    }
    else if (driver_code == 0xC990) {
        write_reg(0x30, 0x0006);
        write_reg(0x31, 0x0101);
        write_reg(0x32, 0x0003);
        write_reg(0x35, 0x0106);
        write_reg(0x36, 0x0B02);
        write_reg(0x37, 0x0302);
        write_reg(0x38, 0x0707);
        write_reg(0x39, 0x0007);
        write_reg(0x3C, 0x0600);
        write_reg(0x3D, 0x020B);
    }

    /* Set GRAM area */
    write_reg(0x50, 0x0000);            /* Horizontal GRAM Start Address      */
    write_reg(0x51, (LCD_HEIGHT - 1));  /* Horizontal GRAM End   Address      */
    write_reg(0x52, 0x0000);            /* Vertical   GRAM Start Address      */
    write_reg(0x53, (LCD_WIDTH - 1));   /* Vertical   GRAM End   Address      */
    write_reg(0x60, 0xA700);            /* Gate Scan Line                     */
    if (driver_code == 0x5408)          /* LCD with touch                     */
        write_reg(0x60, 0xA700);        /* Gate Scan Line                     */
    if (driver_code == 0xC990)          /* LCD without touch                  */
        write_reg(0x60, 0x2700);        /* Gate Scan Line                     */
    write_reg(0x61, 0x0001);            /* NDL,VLE, REV                       */
    write_reg(0x6A, 0x0000);            /* Set scrolling line                 */

    /* Partial Display Control */
    write_reg(0x80, 0x0000);
    write_reg(0x81, 0x0000);
    write_reg(0x82, 0x0000);
    write_reg(0x83, 0x0000);
    write_reg(0x84, 0x0000);
    write_reg(0x85, 0x0000);

    /* Panel Control */
    write_reg(0x90, 0x0010);
    write_reg(0x92, 0x0000);
    write_reg(0x93, 0x0003);
    write_reg(0x95, 0x0110);
    write_reg(0x97, 0x0000);
    write_reg(0x98, 0x0000);

    /* Set GRAM write direction  */
    write_reg(0x03, 0x1038);

    /* 262K color and display ON */
    write_reg(0x07, 0x0137);

    LPC_GPIO4->FIOSET = 0x10000000;
    
    lcd_clear(0x0000);
}

void lcd_clear(uint16_t color)
{
    /* Set draw region */
    write_reg(0x50, (uint16_t)(0));
    write_reg(0x51, (uint16_t)(LCD_HEIGHT - 1));
    write_reg(0x52, (uint16_t)(0));
    write_reg(0x53, (uint16_t)(LCD_WIDTH - 1));
    write_reg(0x20, (uint16_t)(0));
    write_reg(0x21, (uint16_t)(0));

    write_cmd(0x22);
    write_batch_begin();
    write_batch_value(color, LCD_LENGTH);
    write_batch_end();
}

void lcd_rect(uint16_t color, size_t x, size_t y, size_t w, size_t h)
{
    /* Set draw region */
    x = LCD_WIDTH - x - w;
    write_reg(0x50, (uint16_t)(y));
    write_reg(0x51, (uint16_t)(y + h - 1));
    write_reg(0x52, (uint16_t)(x));
    write_reg(0x53, (uint16_t)(x + w - 1));
    write_reg(0x20, (uint16_t)(y));
    write_reg(0x21, (uint16_t)(x));

    /* draw */
    write_cmd(0x22);
    write_batch_begin();
    write_batch_value(color, w * h);
    write_batch_end();
}

void lcd_bitmap(size_t x, size_t y, size_t w, size_t h, const uint16_t *bitmap)
{
    /* Set draw region */
    x = LCD_WIDTH - x - w;
    write_reg(0x50, (uint16_t)(y));
    write_reg(0x51, (uint16_t)(y + h - 1));
    write_reg(0x52, (uint16_t)(x));
    write_reg(0x53, (uint16_t)(x + w - 1));
    write_reg(0x20, (uint16_t)(y));
    write_reg(0x21, (uint16_t)(x));

    /* draw */
    write_cmd(0x22);
    write_batch_begin();
    write_batch_buffer(bitmap, w * h);
    write_batch_end();
}

void lcd_bchar(const struct bfont *fnt, uint16_t fg, uint16_t bg, size_t cx, size_t cy, char ch)
{
    size_t i, j, k;
    size_t numb = fnt->char_width / 8;
    const uint8_t *dat = &fnt->data[(ch - fnt->ascii_offset) * fnt->char_height * numb];

    cx *= fnt->char_width;
    cy *= fnt->char_height;
    cx = LCD_WIDTH - cx - fnt->char_width;

    /* Set draw region */
    write_reg(0x50, (uint16_t)(cy));
    write_reg(0x51, (uint16_t)(cy + fnt->char_height - 1));
    write_reg(0x52, (uint16_t)(cx));
    write_reg(0x53, (uint16_t)(cx + fnt->char_width - 1));
    write_reg(0x20, (uint16_t)(cy));
    write_reg(0x21, (uint16_t)(cx));

    write_cmd(0x22);
    write_batch_begin();

    if(fnt->char_width > __CHAR_BIT__) {
        for(i = 0; i < fnt->char_height; i++) {
            for(j = numb; j != 0; j--)
                for(k = 8; k != 0; k--)
                    write_batch_value((dat[j - 1] & (1 << (k - 1))) ? fg : bg, 1);
            dat += numb;
        }
    }
    else {
        for(i = 0; i < fnt->char_height; i++) {
            for(j = 0; j < numb; j++)
                for(k = 0; k < 8; k++)
                    write_batch_value((dat[j] & (1 << k)) ? fg : bg, 1);
            dat += numb;
        }
    }

    write_batch_end();
}

void lcd_bcline(const struct bfont *fnt, uint16_t bg, size_t cy)
{
    lcd_rect(bg, 0, cy * fnt->char_height, LCD_WIDTH, fnt->char_height);
}

void lcd_bputs(const struct bfont *fnt, uint16_t fg, uint16_t bg, size_t cx, size_t cy, const char *s)
{
    for(; *s; lcd_draw_bchar(fnt, fg, bg, cx++, cy, *s++));
}

void lcd_bprintf(const struct bfont *fnt, uint16_t fg, uint16_t bg, size_t cx, size_t cy, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    lcd_bvprintf(fnt, fg, bg, cx, cy, fmt, ap);
    va_end(ap);
}

void lcd_bvprintf(const struct bfont *fnt, uint16_t fg, uint16_t bg, size_t cx, size_t cy, const char *fmt, va_list ap)
{
    static char buffer[1024] = { 0 };
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    lcd_bputs(fnt, fg, bg, cx, cy, buffer);
}
