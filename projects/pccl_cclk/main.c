#include <lpc17xx.h>
#include <string.h>
#include "lib_joy.h"
#include "lib_lcd.h"
#include "lib_lcd_ibm_8x16.h"
#include "lib_led.h"

/* choices in the menu */
#define CHOICE_MIN      0
#define CHOICE_MAX      3
#define CHOICE_MSEL     0
#define CHOICE_NSEL     1
#define CHOICE_CCLKSEL  2
#define CHOICE_APPLY    3

/* cursor pointing at choices */
#define CURSOR_COL_BASE 0
#define CURSOR_ROW_BASE 0
#define CURSOR_ROW_MAX  4

/* text information about the choice */
#define INFO_COL_BASE   2
#define INFO_ROW_BASE   0

/* value of the choice */
#define VALUE_COL_BASE  12
#define VALUE_ROW_BASE  0

/* frequency values */
#define FREQ_ROW_BASE   5
#define FINF_COL_BASE   0
#define FREQ_COL_BASE   2
#define FREQ_REAL       0
#define FREQ_CALCED     1

/* status values */
#define STAT_ROW_BASE   8
#define STAT_COL_BASE   1
#define STAT_READY      0
#define STAT_INVAL      1

/* limit values */
#define MSEL_MAX        32767
#define NSEL_MAX        255
#define CCLKSEL_MAX     255
#define CCLK_MIN        10000000
#define CCLK_MAX        100000000
#define FCCO_MIN        275000000
#define FCCO_MAX        550000000

struct cursor {
    unsigned int choice;
    unsigned char value;
};

struct core_config {
    unsigned int msel;
    unsigned int nsel;
    unsigned int cclksel;
    float real_freq;
};

static void set_choice(struct cursor *cur, unsigned int choice)
{
    GLCD_DisplayChar(CURSOR_ROW_BASE + cur->choice, CURSOR_COL_BASE, ' ');
    GLCD_DisplayChar(CURSOR_ROW_BASE + choice, CURSOR_COL_BASE, cur->value);
    cur->choice = choice;
}

static void print_choice_info(unsigned int choice)
{
    char buffer[64] = { 0 };
    switch(choice) {
        case CHOICE_MSEL:
            strncpy(buffer, "MSEL", sizeof(buffer));
            break;
        case CHOICE_NSEL:
            strncpy(buffer, "NSEL", sizeof(buffer));
            break;
        case CHOICE_CCLKSEL:
            strncpy(buffer, "CCLKSEL", sizeof(buffer));
            break;
        case CHOICE_APPLY:
            strncpy(buffer, "APPLY", sizeof(buffer));
            break;
        default:
            return;
    }
    
    lcd_bprintf(&IBM_8x16, 0xFFFF, 0x0000, INFO_ROW_BASE + choice, INFO_COL_BASE, "%s", (unsigned char *)buffer);
}

static void print_choice_value(unsigned int choice, unsigned int value)
{
    lcd_bprintf(&IBM_8x16, 0xFFFF, 0x0000, VALUE_ROW_BASE + choice, VALUE_COL_BASE, "%-5u", value + 1);
}

static void print_frequency(unsigned int id, float value)
{
    char prefix;
    switch(id) {
        case FREQ_REAL:
            prefix = 'R';
            break;
        case FREQ_CALCED:
            prefix = 'C';
            break;
        default:
            return;
    }

    lcd_bprintf(&IBM_8x16, 0xFFFF, 0x0000, FREQ_ROW_BASE + id, FINF_COL_BASE, "%c", prefix);
    lcd_bprintf(&IBM_8x16, 0xFFFF, 0x0000, FREQ_ROW_BASE + id, FREQ_COL_BASE, "%-12.03f MHz", value * 1.0e-6f);
}

static void print_status(unsigned int status)
{
    char buffer[64] = { 0 };
    switch(status) {
        case STAT_READY:
            strncpy(buffer, "READY", sizeof(buffer));
            break;
        case STAT_INVAL:
            strncpy(buffer, "INVAL", sizeof(buffer));
            break;
        default:
            return;
    }
    
    lcd_bprintf(&IBM_8x16, 0xFFFF, 0x0000, STAT_ROW_BASE, STAT_COL_BASE, "%s", buffer);
}

static void get_config(struct core_config *conf)
{
    conf->msel = (unsigned int)(LPC_SC->PLL0CFG & 0x00007FFF);
    conf->nsel = (unsigned int)((LPC_SC->PLL0CFG & 0x00FF0000) >> 16);
    conf->cclksel = (unsigned int)(LPC_SC->CCLKCFG & 0x000000FF);
}

static float get_config_freq(const struct core_config *conf)
{
    float fin, fcco;

    switch(LPC_SC->CLKSRCSEL & 0x03) {
        case 0:
            fin = 4.0e6f;
            break;
        case 1:
            fin = 12.0e6f;
            break;
        case 2:
            fin = 32.768e3f;
            break;
        default:
            return 0.0f;
    }

    fcco = (2.0f * (float)(conf->msel + 1) * fin) / (float)(conf->nsel + 1);
    if(fcco < (float)(FCCO_MIN) || fcco > (float)(FCCO_MAX))
        return 0.0f;
    
    return fcco / (float)(conf->cclksel + 1);
}

static void print_config(const struct core_config *conf)
{
    print_choice_info(CHOICE_MSEL);
    print_choice_info(CHOICE_NSEL);
    print_choice_info(CHOICE_CCLKSEL);
    print_choice_info(CHOICE_APPLY);
    
    print_choice_value(CHOICE_MSEL, conf->msel);
    print_choice_value(CHOICE_NSEL, conf->nsel);
    print_choice_value(CHOICE_CCLKSEL, conf->cclksel);
    
    print_frequency(FREQ_REAL, conf->real_freq);
    print_frequency(FREQ_CALCED, get_config_freq(conf));
}

static void apply_config(struct core_config *conf)
{
    uint32_t scratch;
    float freq = get_config_freq(conf);
    if(freq <= (float)(CCLK_MIN) || freq > (float)(CCLK_MAX)) {
        get_config(conf);
        print_status(STAT_INVAL);
        return;
    }
    
    scratch = 0;
    scratch |= (uint32_t)(conf->msel & 0x00007FFF);
    scratch |= (uint32_t)((conf->nsel << 16) & 0x00FF0000);
    LPC_SC->PLL0CFG = scratch;
    
    scratch = 0;
    scratch |= (uint32_t)(conf->cclksel & 0x000000FF);
    LPC_SC->CCLKCFG = scratch;
    
    conf->real_freq = get_config_freq(conf);
    
    print_status(STAT_READY);
}

static void update_config(struct core_config *conf, unsigned int choice, int add)
{
    switch(choice) {
        case CHOICE_MSEL:
            if(add && conf->msel < MSEL_MAX)
                conf->msel++;
            else if(!add && conf->msel > 0)
                conf->msel--;
            break;
        case CHOICE_NSEL:
            if(add && conf->nsel < NSEL_MAX)
                conf->nsel++;
            else if(!add && conf->nsel > 0)
                conf->nsel--;
            break;
        case CHOICE_CCLKSEL:
            if(add && conf->cclksel < CCLKSEL_MAX)
                conf->cclksel++;
            else if(!add && conf->cclksel > 0)
                conf->cclksel--;
            break;
        case CHOICE_APPLY:
            apply_config(conf);
            break;
    }
}

int __attribute__((noreturn)) main(void)
{
    unsigned int bit;
    unsigned long t, w;
    struct cursor cur = { 0 };
    struct core_config conf = { 0 };
    
    SystemInit();
    joy_init();
    led_init();
    lcd_init();

    lcd_clear(0x0000);
    
    cur.choice = CHOICE_MSEL;
    cur.value = '>';

    /* query config */
    get_config(&conf);

    /* query real frequency */
    conf.real_freq = get_config_freq(&conf);
    
    /* force the cursor to redraw */
    set_choice(&cur, CHOICE_MSEL);

    /* redraw interface */
    print_config(&conf);

    /* redraw status message */
    print_status(STAT_READY);

    bit = 0;
    t = 0;
    w = 0;

    for(;; t++) {
        joy_query();

        /* Move cursor down */
        if(joy_pressed(JOY_UP) && cur.choice > CHOICE_MIN) {
            print_status(STAT_READY);
            set_choice(&cur, cur.choice - 1);
        }

        /* Move cursor up */
        if(joy_pressed(JOY_DN) && cur.choice < CHOICE_MAX) {
            print_status(STAT_READY);
            set_choice(&cur, cur.choice + 1);
        }

        /* Decrease or apply */
        if(joy_pressed(JOY_LF)) {
            print_status(STAT_READY);
            update_config(&conf, cur.choice, 0);
            print_config(&conf);
        }

        /* Increase or apply */
        if(joy_pressed(JOY_RT)) {
            print_status(STAT_READY);
            update_config(&conf, cur.choice, 1);
            print_config(&conf);
        }

        joy_store();

        if(t < w) {
            /* Tick-based delay */
            continue;
        }

        /* Oleg Vasiliyevich asked for this. */
        /* Practically demonstrates frequency changes */
        led_set((uint8_t)(1U << (bit++ % 8)));

        w = t + 50000;
    }
}
