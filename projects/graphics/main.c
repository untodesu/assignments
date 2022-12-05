#include <lpc17xx.h>
#include <string.h>
#include "lib_joy.h"
#include "lib_lcd.h"
#include "lib_lcd_ibm_8x16.h"
#include "lib_lcd_ibm_8x8.h"
#include "bmp.h"

/* Screen modes */
#define SCREEN_MENU     0x00
#define SCREEN_TEXT     0x01
#define SCREEN_FIGURE   0x02
#define SCREEN_BITMAP   0x03
#define SCREEN_COUNT    0x04

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

typedef void(*screen_t)(void);
static screen_t screens[SCREEN_COUNT] = { 0 };
static unsigned int current_screen = SCREEN_MENU;
static unsigned long ticks = 0;

static void set_screen(unsigned int screen)
{
    ticks = 0;
    current_screen = screen;
    lcd_clear(0x0000);
}

#define MENU_BEGIN (SCREEN_TEXT)
#define MENU_INFOLN (SCREEN_COUNT + 1)
#define MENU_TIMELN (SCREEN_COUNT + 3)
static void draw_menu(void)
{
    static const char *menu_items[SCREEN_COUNT] = {
        [SCREEN_MENU]   = "Sussy Baka", /* unused, redundancy */
        [SCREEN_TEXT]   = "Text",
        [SCREEN_FIGURE] = "Figure",
        [SCREEN_BITMAP] = "Bitmap",
    };
    
    static const char *menu_infos[SCREEN_COUNT] = {
        [SCREEN_MENU]   = "Sussy Amogus", /* unused, redundancy */
        [SCREEN_TEXT]   = "Print some text",
        [SCREEN_FIGURE] = "Draw a figure",
        [SCREEN_BITMAP] = "Draws a picture",
    };
    
    static unsigned int i;
    static unsigned int choice = MENU_BEGIN;
    static char str[128] = { 0 };
    static int blinks = 1;

    if(joy_pressed(JOY_UP)) {
        GLCD_ClearLn(MENU_INFOLN);
        blinks = 1;
        choice--;
    }

    if(joy_pressed(JOY_DN)) {
        GLCD_ClearLn(MENU_INFOLN);
        blinks = 1;
        choice++;
    }

    if(joy_pressed(JOY_LF) || joy_pressed(JOY_RT)) {
        set_screen(choice);
        return;
    }
    
    if((ticks % 10) == 0)
        blinks = !blinks;
    
    choice = MACRO_MAX(choice, MENU_BEGIN);
    choice = MACRO_MIN(choice, SCREEN_COUNT - 1);
    
    for(i = MENU_BEGIN; i < SCREEN_COUNT; i++) {
        memset(str, 0, sizeof(str));
        strncat_k(str, (blinks && (i == choice)) ? "> " : "  ", sizeof(str));
        strncat_k(str, menu_items[i], sizeof(str));
        cfont_puts(&IBM_8x16, 1, i, str);
    }
    
    cfont_puts(&IBM_8x16, 1, MENU_INFOLN, menu_infos[choice]);
    
    snprintf(str, sizeof(str), "0x%-16lX", ticks);
    cfont_puts(&IBM_8x16, 1, MENU_TIMELN, str);
}

static void draw_text(void)
{
    /* One line in 8x16 font equals to two in 8x8 */
    #define NLINES_8x8  6
    #define NLINES_8x16 9


    static const char *lines_8x8[NLINES_8x8] = {
        "Lorem ipsum dolor sit amet, consectetur ",
        "adipiscing elit. Donec porta elit quis e",
        "nim iaculis, eu lobortis ante vestibulum",
        ". Fusce condimentum massa vitae mi imper",
        "diet imperdiet. Etiam auctor tristique u",
        "rna, nec ornare erat auctor a. Pellentes",
    };

    static const char *lines_8x16[NLINES_8x16] = {
        "que maximus, metus nec aliquam ullamcorp",
        "er, odio metus aliquam tellus, vel pulvi",
        "nar enim lectus sed dolor. Sed facilisis",
        " iaculis sem, id iaculis elit. Nunc vari",
        "us arcu non mollis eleifend. Praesent ve",
        "hicula vel ante eu sagittis. Ut a elit i",
        "d eros interdum vestibulum in sed arcu. ",
        "Phasellus ac cursus nunc. Maecenas ut li",
        "gula nisi. Aenean vitae imperdiet justo.",
    };

    if(joy_pressed(JOY_LF) || joy_pressed(JOY_RT)) {
        set_screen(SCREEN_MENU);
        return;
    }

    unsigned int i;
    for(i = 0; i < NLINES_8x8; lcd_bvprintf(&IBM_8x8, 0xFFFF, 0x0000, 0, i, "%s", lines_8x8[i]), i++);
    for(i = 0; i < NLINES_8x16; lcd_bvprintf(&IBM_8x16, 0xFFFF, 0x0000, 0, i + NLINES_8x8 / 2 + 1, "%s", lines_8x16[i]), i++);
}

#define FIG_FRAME 16
#define FIG_WIDTH (LCD_WIDTH - FIG_FRAME - FIG_FRAME)
#define FIG_HEIGHT ((LCD_HEIGHT - FIG_FRAME - FIG_FRAME) / 3)
static void draw_figure(void)
{
    if(joy_pressed(JOY_LF) || joy_pressed(JOY_RT)) {
        set_screen(SCREEN_MENU);
        return;
    }

    /* Ural Republic flag */
    lcd_rect(0xFFFF, FIG_FRAME, FIG_FRAME, FIG_WIDTH, FIG_HEIGHT);
    lcd_rect(0x07E0, FIG_FRAME, FIG_HEIGHT + FIG_FRAME, FIG_WIDTH, FIG_HEIGHT);
    lcd_rect(0x2104, FIG_FRAME, FIG_HEIGHT + FIG_HEIGHT + FIG_FRAME, FIG_WIDTH, FIG_HEIGHT);
}

static void draw_bitmap(void)
{
    if(joy_pressed(JOY_LF) || joy_pressed(JOY_RT)) {
        set_screen(SCREEN_MENU);
        return;
    }

    lcd_bitmap(0, 0, BMP_WIDTH, BMP_HEIGHT, bmp);
}

int __attribute__((noreturn)) main(void)
{
    SystemInit();
    joy_init();
    lcd_init();

    /* Setup screen redraw callbacks */
    memset(screens, 0, sizeof(screens));
    screens[SCREEN_MENU] = &draw_menu;
    screens[SCREEN_TEXT] = &draw_text;
    screens[SCREEN_FIGURE] = &draw_figure;
    screens[SCREEN_BITMAP] = &draw_bitmap;

    /* Start at MENU */
    set_screen(SCREEN_MENU);

    for(;; ticks++) {
        joy_query();
        if(screens[current_screen])
            screens[current_screen]();
        joy_store();
    }
}
