#include <ctype.h>
#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib_lcd.h"
#include "lib_lcd_ibm_8x16.h"
#include "lib_led.h"
#include "lib_uart.h"

#define TIMER_CX 0
#define TIMER_CY 3

#define REPL_PROMPT '@'
#define REPL_MAX_ARGV 4
#define REPL_ARGV_SIZE 32
#define REPL_BUFFER_SIZE (REPL_MAX_ARGV * REPL_ARGV_SIZE)
typedef char repl_arg_t[REPL_ARGV_SIZE];
static char repl_buffer[REPL_BUFFER_SIZE] = { 0 };
static size_t repl_buffer_writepos = 0;
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
    timer_millis += 5;
}

static void repl_eval(void)
{
    size_t i, j, argc;
    repl_arg_t argv[REPL_MAX_ARGV] = { 0 };
    unsigned long arg_numeric;

    /* skip trailing whitespace */
    for(i = 0; isspace(repl_buffer[i]); i++);
    
    j = 0;
    argc = 0;
    memset(argv, 0, sizeof(argv));
    for(; i < REPL_BUFFER_SIZE && repl_buffer[i] && repl_buffer[i] != '\r'; i++) {
        if(argc >= REPL_MAX_ARGV) {
            /* Overflow!!! */
            break;
        }
        
        if((isspace(repl_buffer[i]) && j) || (j >= (REPL_ARGV_SIZE - 1))) {
            j = 0;
            argc++;
            continue;
        }
        
        if(isspace(repl_buffer[i])) {
            /* ignore any other whitespace */
            continue;
        }
        
        argv[argc][j++] = repl_buffer[i];
    }
    
    argc++;
    
    if(!argv[0][0]) {
        /* ignore empty commands */
        return;
    }
    
    /* led.u.s -- control LED stack */
    if(!strcmp(argv[0], "led")) {
        if(argc < 3) {
            uart_printf("%s: not enough arguments (%zu)\r\n", argv[0], argc);
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

        uart_printf("%s: unknown argument %s\r\n", argv[0], argv[2]);
        return;
    }

    /* color.s.x -- control colors */
    if(!strcmp(argv[0], "color")) {
        if(argc < 3) {
            uart_printf("%s: not enough arguments\r\n", argv[0]);
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

        uart_printf("%s: unknown argument %s\r\n", argv[0], argv[1]);
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
            uart_printf("%s: not enough arguments\r\n", argv[0]);
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

        uart_printf("%s: unknown argument %s\r\n", argv[0], argv[1]);
        return;
    }
    
    /* help -- print help */
    if(!strcmp(argv[0], "help")) {
        uart_printf("Available commands:\r\n");
        uart_printf(" led [1..8]      - Query LED state\r\n");
        uart_printf(" led [1..8] on   - Turn LED on\r\n");
        uart_printf(" led [1..8] off  - Turn LED off\r\n");
        uart_printf(" color bg [XXXX] - Set LCD's background color\r\n");
        uart_printf(" color fg [XXXX] - Set LCD's foreground color\r\n");
        uart_printf(" clear           - Clear LCD\r\n");
        uart_printf(" timer stop      - Stop on-screen timer\r\n");
        uart_printf(" timer start     - Start on-screen timer (may be still hidden)\r\n");
        uart_printf(" timer restart   - Restart on-screen timer\r\n");
        uart_printf(" timer show      - Show on-screen timer\r\n");
        uart_printf(" timer hide      - Hide on-screen timer\r\n");
        uart_printf(" help            - Print this message\r\n");
        return;
    }

    uart_printf("unknown command: %s\r\n", argv[0]);
    for(i = 0; i < argc; i++)
        uart_printf("[%2zu] '%s'\r\n", i, argv[i]);
    return;
}

static void repl_append(const char *s)
{
    char *p;

    /* Append to the buffer */
    while(*s) {
        if(repl_buffer_writepos >= REPL_BUFFER_SIZE) {
            uart_printf("\r\ninput buffer overflow!!!\r\n%c ", REPL_PROMPT);
            memset(repl_buffer, 0, sizeof(repl_buffer));
            repl_buffer_writepos = 0;
            return;
        }

        repl_buffer[repl_buffer_writepos++] = *s++;
    }
    
    /* There is a CR character, treat it as ENTER */
    if((p = strchr(repl_buffer, '\r')) != NULL) {
        uart_printf("\r\n");

        /* Cut the buffer at CR */
        p[0] = 0x00;

        repl_eval();
        uart_printf("%c ", REPL_PROMPT);

        memset(repl_buffer, 0, sizeof(repl_buffer));
        repl_buffer_writepos = 0;
    }
}

extern uint32_t SystemFrequency;
int __attribute__((noreturn)) main(void)
{
    unsigned long t;
    size_t ms, s, m, h;
    size_t input_count, i;
    char input[16] = { 0 };

    SystemInit();
    led_init();
    lcd_init();
    uart_init(57600, UART_WORD_8 | UART_PR_EVN);
    uart_set_discard("\b\x1B\x7F");
    uart_puts("\r\n");
    uart_printf("board running at %.02f MHz\r\n", ((float)SystemFrequency / 1.0e6f));

    lcd_clear(color_bg);

    SysTick->LOAD = (SystemFrequency / 200) - 1;
    SysTick->VAL = 0x00000000;
    SysTick->CTRL = (1 << 0) | (1 << 1) | (1 << 2);
    
    uart_printf("%c ", REPL_PROMPT);
    for(t = 0;; t++) {
        input_count = uart_read(input, sizeof(input));
        if(input_count) {
            uart_write(input, input_count);
            repl_append(input);
            lcd_bcline(&IBM_8x16, color_bg, 0);
            for(i = 0; i < input_count; i++) {
                /* dump input in form of hex bytes */
                lcd_bprintf(&IBM_8x16, color_fg, color_bg, i * 3, 0, "%02X", input[i]);
            }
            
            /* clear input buffer */
            memset(input, 0, sizeof(input));
        }

        if(timer_visible && !(t % 5000)) {
            ms = timer_millis % 1000;
            s = timer_millis / 1000;
            m = s / 60;
            h = m / 60;
            lcd_bprintf(&IBM_8x16, color_fg, color_bg, TIMER_CX, TIMER_CY, "%c %002zu:%02zu:%02zu.%03zu", timer_active ? '>' : '#', h, m % 60, s % 60, ms);
        }
    }
}
