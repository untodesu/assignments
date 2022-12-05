#ifndef A2446573_3F8C_4717_9270_4C5ED29CBB53
#define A2446573_3F8C_4717_9270_4C5ED29CBB53

#define JOY_UP 23
#define JOY_RT 24
#define JOY_DN 25
#define JOY_LF 26

void joy_init(void);
void joy_query(void);
void joy_store(void);
int joy_pressed(unsigned long button);

#endif /* A2446573_3F8C_4717_9270_4C5ED29CBB53 */
