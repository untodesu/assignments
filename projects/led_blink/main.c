#include <lpc17xx.h>
#include "lib_joystick.h"

#define MASK_MIN 1
#define MASK_MAX 8
#define DELAY_DT 50000
#define DELAY_MIN 50000
#define DELAY_MAX 500000

static void gpio_set_bit(volatile LPC_GPIO_TypeDef *gpio, unsigned int bit, bool set)
{
    volatile uint32_t *r = set
        ? &gpio->FIOSET
        : &gpio->FIOCLR;
    r[0] |= (1U << bit);
}

int __attribute__((noreturn)) main(void)
{
    unsigned char v;
    unsigned int bit;
    unsigned int mask;
    unsigned long t, w;
    unsigned long delay;
    struct joystick j = { 0 };

    SystemInit();

    joy_init(&j);

    /* Set P1.XX (LED stack) for OUTPUT */
    LPC_GPIO1->FIODIR |= (1U << 28);
    LPC_GPIO1->FIODIR |= (1U << 29);
    LPC_GPIO1->FIODIR |= (1U << 31);
    
    /* Set P2.XX (LED stack) for OUTPUT */
    LPC_GPIO2->FIODIR |= (1U << 2);
    LPC_GPIO2->FIODIR |= (1U << 3);
    LPC_GPIO2->FIODIR |= (1U << 4);
    LPC_GPIO2->FIODIR |= (1U << 5);
    LPC_GPIO2->FIODIR |= (1U << 6);
    
    bit = 0;
    mask = MASK_MIN;
    delay = DELAY_MIN;
    for(t = 0, w = 0;; t++) {
        joy_query(&j);

        if(joy_pressed(&j, JOYSTICK_UP) && mask < MASK_MAX)
            mask++;
        if(joy_pressed(&j, JOYSTICK_DN) && mask > MASK_MIN)
            mask--;
        if(joy_pressed(&j, JOYSTICK_LF) && delay > DELAY_MIN)
            delay -= DELAY_DT;
        if(joy_pressed(&j, JOYSTICK_RT) && delay < DELAY_MAX)
            delay += DELAY_DT;

        joy_store(&j);

        if(t < w) {
            /* Tick-based delay */
            continue;
        }

        /* Convert counter to a bitmask */
        v = (unsigned char)(1U << (bit++ % mask));
        gpio_set_bit(LPC_GPIO1, 28, v & (1 << 0));
        gpio_set_bit(LPC_GPIO1, 29, v & (1 << 1));
        gpio_set_bit(LPC_GPIO1, 31, v & (1 << 2));
        gpio_set_bit(LPC_GPIO2, 2, v & (1 << 3));
        gpio_set_bit(LPC_GPIO2, 3, v & (1 << 4));
        gpio_set_bit(LPC_GPIO2, 4, v & (1 << 5));
        gpio_set_bit(LPC_GPIO2, 5, v & (1 << 6));
        gpio_set_bit(LPC_GPIO2, 6, v & (1 << 7));

        w = t + delay;
    }
}
