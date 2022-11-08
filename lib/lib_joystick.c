#include "joystick.h"

void joy_init(struct joystick *j)
{
    j->cs = 0;
    j->ps = 0;

    /* Set P1.XX (joystick) for INPUT */
    LPC_GPIO1->FIODIR &= ~JOYSTICK_UP;
    LPC_GPIO1->FIODIR &= ~JOYSTICK_RT;
    LPC_GPIO1->FIODIR &= ~JOYSTICK_DN;
    LPC_GPIO1->FIODIR &= ~JOYSTICK_LF;
}

void joy_query(struct joystick *j)
{
    j->cs = (unsigned long)(LPC_GPIO1->FIOPIN);
}

void joy_store(struct joystick *j)
{
    j->ps = j->cs;
}

bool joy_pressed(struct joystick *j, unsigned long button)
{
    unsigned long cbit = j->cs & button;
    unsigned long pbit = j->ps & button;
    if((cbit != pbit) && !cbit)
        return true;
    return false;
}
