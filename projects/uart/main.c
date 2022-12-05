#include <ctype.h>
#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib_lcd.h"
#include "lib_lcd_ibm_8x16.h"
#include "lib_led.h"
#include "lib_uart.h"

/* https://github.com/kaneru-os/kaneru/blob/master/kernel/lib/strnlen.c */
size_t strnlen(const char *restrict s, size_t n)
{
    size_t i;
    for(i = 0; *s++ && i < n; i++);
    return i;
}

/* https://github.com/kaneru-os/kaneru/blob/master/kernel/lib/strncat_k.c */
char *strncat_k(char *restrict s1, const char *restrict s2, size_t n)
{
    size_t nc;
    char *save = s1;
    while(*s1 && n--)
        s1++;
    nc = strnlen(s2, --n);
    s1[nc] = 0;
    memcpy(s1, s2, nc);
    return save;
}

#define TIMER_CX 1
#define TIMER_CY 3

#define REPL_PROMPT '@'
#define REPL_MAX_ARGV 4
#define REPL_ARGV_SIZE 32
#define REPL_BUFFER_SIZE (REPL_MAX_ARGV * REPL_ARGV_SIZE)
static char repl_buffer[REPL_BUFFER_SIZE] = { 0 };
static uint16_t color_bg = 0x0000;
static uint16_t color_fg = 0xFFFF;
static uint8_t active_leds = 0x00;
static size_t timer_millis = 0;
static int timer_visible = 0;
static int timer_active = 0;

void __attribute__((used)) SysTick_Handler(void)
{
    if(!timer_active)
        return;
    timer_millis++;
}

static void repl_eval(void)
{
    size_t i, j, argc;
    char argv[REPL_ARGV_SIZE][REPL_MAX_ARGV] = { 0 };
    unsigned long arg_numeric;

    j = 0;
    argc = 0;
    memset(argv, 0, sizeof(argv));
    for(i = 0; i < REPL_BUFFER_SIZE && repl_buffer[i] && repl_buffer[i] != '\r'; i++) {
        if(isspace(repl_buffer[i])) {
            /* ignore whitespace */
            continue;
        }
        
        /* go to the next argument either when we encounter
         * a separator character or we run out of space. */
        if(repl_buffer[i] == '.' || j >= (REPL_ARGV_SIZE - 1)) {
            j = 0;
            argc++;
            continue;
        }

        argv[argc][j++] = repl_buffer[i];
    }

    /* led.u.s -- control LED stack */
    if(!strcmp(argv[0], "led")) {
        if(argc < 3) {
            uart_printf("%s: not enough argumentsr\r\n", argv[0]);
            return;
        }

        arg_numeric = strtoul(argv[1], NULL, 10);
        if(arg_numeric == 0 || arg_numeric > 8) {
            uart_printf("%s: argument out of range (value: %lu, range: [1..8])\r\n", argv[0], arg_numeric);
            return;
        }

        if(!strcmp(argv[2], "on")) {
            active_leds |= (1 << (arg_numeric - 1));
            led_set(active_leds);
            return;
        }

        if(!strcmp(argv[2], "off")) {
            active_leds &= ~(1 << (arg_numeric - 1));
            led_set(active_leds);
            return;
        }

        uart_printf("%s: unknown argument %s", argv[0], argv[2]);
        return;
    }

    /* color.s.x -- control colors */
    if(!strcmp(argv[0], "color")) {
        if(argc < 3) {
            uart_printf("%s: not enough argumentsr\r\n", argv[0]);
            return;
        }

        arg_numeric = strtoul(argv[2], NULL, 16);
        if(arg_numeric > 0xFFFF) {
            uart_printf("%s: argument out of range (value: %#04lX, range: [0000..FFFF])\r\n", argv[0], arg_numeric);
            return;
        }

        if(!strcmp(argv[1], "bg")) {
            color_bg = (uint16_t)(arg_numeric);
            return;
        }

        if(!strcmp(argv[1], "fg")) {
            color_fg = (uint16_t)(arg_numeric);
            return;
        }

        uart_printf("%s: unknown argument %s", argv[0], argv[1]);
        return;
    }

    /* clear -- clear screen */
    if(!strcmp(argv[0], "clear")) {
        lcd_clear(color_bg);
        return;
    }

    /* timer.s.s -- control timer */
    if(!strcmp(argv[0], "timer")) {
        if(argc < 2) {
            uart_printf("%s: not enough argumentsr\r\n", argv[0]);
            return;
        }

        if(!strcmp(argv[1], "stop")) {
            timer_active = 0;
            return;
        }

        if(!strcmp(argv[1], "start")) {
            timer_active = 1;
            return;
        }

        if(!strcmp(argv[1], "restart")) {
            timer_millis = 0;
            timer_active = 1;
            lcd_bcline(&IBM_8x16, color_bg, TIMER_CY);
            return;
        }

        if(!strcmp(argv[1], "clear")) {
            timer_millis = 0;
            lcd_bcline(&IBM_8x16, color_bg, TIMER_CY);
            return;
        }

        if(!strcmp(argv[1], "show")) {
            timer_visible = 1;
            lcd_bcline(&IBM_8x16, color_bg, TIMER_CY);
            return;
        }

        if(!strcmp(argv[1], "hide")) {
            timer_visible = 0;
            lcd_bcline(&IBM_8x16, color_bg, TIMER_CY);
            return;
        }

        uart_printf("%s: unknown argument %s", argv[0], argv[1]);
        return;
    }

    uart_printf("%s: unknown command\r\n", argv[0]);
    return;
}

static void repl_append(const char *s)
{
    char *p;

    /* Append to the buffer */
    strncat_k(repl_buffer, s, sizeof(repl_buffer));

    /* There is a CR character, treat it as ENTER */
    if((p = strchr(repl_buffer, '\r')) != NULL) {
        uart_printf("\r\n");

        /* Cut the buffer at CR */
        p[0] = 0x00;

        repl_eval();
        uart_printf("%c ", REPL_PROMPT);

        /* Forget the evaluated command */
        memset(repl_buffer, 0, sizeof(repl_buffer));
    }
}

extern uint32_t SystemFrequency;
int __attribute__((noreturn)) main(void)
{
    unsigned long t;
    size_t ms, s, m, h;
    size_t input_count;
    char input[16] = { 0 };

    SystemInit();
    led_init();
    lcd_init();
    uart_init(57600, UART_WORD_8 | UART_PR_EVN);

    lcd_clear(color_bg);

    SysTick->LOAD = (SystemFrequency / 1000) - 1;
    SysTick->VAL = 0x00000000;
    SysTick->CTRL = (1 << 0) | (1 << 1) | (1 << 2);
    
    uart_printf("\r\n");
    uart_printf("Board running at %f MHz\r\n", ((float)SystemFrequency / 1.0e6f));
    uart_printf("\r\n");

    uart_printf("%c ", REPL_PROMPT);
    for(t = 0;; t++) {
        input_count = uart_read(input, sizeof(input));
        if(input_count) {
            uart_write(input, input_count);
            repl_append(input);
        }

        if(timer_visible && t % 5000 == 0) {
            ms = timer_millis % 1000;
            s = timer_millis / 1000;
            m = s / 60;
            h = m / 60;
            lcd_bprintf(&IBM_8x16, color_fg, color_bg, TIMER_CX, TIMER_CY, "%c %002zu:%02zu:%02zu.%03zu", timer_active ? '>' : '#', h, m % 60, s % 60, ms);
        }
    }

    return 0;
}
