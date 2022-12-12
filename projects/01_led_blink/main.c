#include <lpc17xx.h>
#include "lib_joy.h"
#include "lib_led.h"

#define MASK_MIN 1
#define MASK_MAX 8
#define DELAY_DT 50000
#define DELAY_MIN 50000
#define DELAY_MAX 500000

int __attribute__((noreturn)) main(void)
{
    unsigned int bit;
    unsigned int mask;
    unsigned long t, w;
    unsigned long delay;

    SystemInit();
    joy_init();
    led_init();

    bit = 0;
    mask = MASK_MAX;
    delay = DELAY_MIN;
    for(t = 0, w = 0;; t++) {
        joy_query();

        if(joy_pressed(JOY_UP) && mask < MASK_MAX)
            mask++;
        if(joy_pressed(JOY_DN) && mask > MASK_MIN)
            mask--;
        if(joy_pressed(JOY_LF) && delay > DELAY_MIN)
            delay -= DELAY_DT;
        if(joy_pressed(JOY_RT) && delay < DELAY_MAX)
            delay += DELAY_DT;

        joy_store();

        if(t < w) {
            /* Tick-based delay */
            continue;
        }

        /* Do a running light effect */
        led_set((uint8_t)(1U << (bit++ % mask)));

        w = t + delay;
    }
}
