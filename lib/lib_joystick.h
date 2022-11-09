#ifndef A2446573_3F8C_4717_9270_4C5ED29CBB53
#define A2446573_3F8C_4717_9270_4C5ED29CBB53
#include <lpc17xx.h>
#include <stdbool.h>

/* Joystick attached to GPIO pins */
#define JOYSTICK_UP (1U << 23)
#define JOYSTICK_RT (1U << 24)
#define JOYSTICK_DN (1U << 25)
#define JOYSTICK_LF (1U << 26)

struct joystick {
    unsigned long cs;
    unsigned long ps;
};

void joy_init(struct joystick *j);
void joy_query(struct joystick *j);
void joy_store(struct joystick *j);
bool joy_pressed(struct joystick *j, unsigned long button);

#endif /* A2446573_3F8C_4717_9270_4C5ED29CBB53 */
