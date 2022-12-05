#include <lpc17xx.h>
#include <stdint.h>
#include "lib_joy.h"

static uint32_t joy_cs = 0;
static uint32_t joy_ps = 0;

void joy_init(void)
{
    joy_cs = 0;
    joy_ps = 0;

    /* Set P1.XX (joystick) for INPUT */
    LPC_GPIO1->FIODIR &= ~(1 << JOY_UP);
    LPC_GPIO1->FIODIR &= ~(1 << JOY_RT);
    LPC_GPIO1->FIODIR &= ~(1 << JOY_DN);
    LPC_GPIO1->FIODIR &= ~(1 << JOY_LF);
}

void joy_query(void)
{
    joy_cs = LPC_GPIO1->FIOPIN;
}

void joy_store(void)
{
    joy_ps = joy_cs;
}

int joy_pressed(unsigned long button)
{
    unsigned long cbit = joy_cs & (1 << button);
    unsigned long pbit = joy_ps & (1 << button);
    if((cbit != pbit) && !cbit)
        return 1;
    return 0;
}
