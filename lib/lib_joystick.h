#ifndef LIB_JOYSTICK_H__
#define LIB_JOYSTICK_H__ 1
#include <lpc17xx.h>
#include <stdbool.h>

/* Joystick attached to GPIO pins */
#define JOYSTICK_UP (1 << 23)
#define JOYSTICK_RT (1 << 24)
#define JOYSTICK_DN (1 << 25)
#define JOYSTICK_LF (1 << 26)

struct joystick {
    unsigned long cs;
    unsigned long ps;
};

void joy_init(struct joystick *j);
void joy_query(struct joystick *j);
void joy_store(struct joystick *j);
bool joy_pressed(struct joystick *j, unsigned long button);

#endif /* LIB_JOYSTICK_H__ */
