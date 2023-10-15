#include <lpc17xx.h>
#include "lib_led.h"

static void gpio_set_bit(volatile LPC_GPIO_TypeDef *gpio, unsigned int bit, int set)
{
    volatile uint32_t *r = set
        ? &gpio->FIOSET
        : &gpio->FIOCLR;
    r[0] |= (1U << bit);
}

void led_init(void)
{
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
}

void led_set(uint8_t bits)
{
    gpio_set_bit(LPC_GPIO1, 28, bits & (1 << 0));
    gpio_set_bit(LPC_GPIO1, 29, bits & (1 << 1));
    gpio_set_bit(LPC_GPIO1, 31, bits & (1 << 2));
    gpio_set_bit(LPC_GPIO2, 2, bits & (1 << 3));
    gpio_set_bit(LPC_GPIO2, 3, bits & (1 << 4));
    gpio_set_bit(LPC_GPIO2, 4, bits & (1 << 5));
    gpio_set_bit(LPC_GPIO2, 5, bits & (1 << 6));
    gpio_set_bit(LPC_GPIO2, 6, bits & (1 << 7));
}
