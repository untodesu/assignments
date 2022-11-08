#include <lpc17xx.h>

/* Joystick attached to GPIO pins */
#define JOYSTICK_UP (1 << 23)
#define JOYSTICK_RT (1 << 24)
#define JOYSTICK_DN (1 << 25)
#define JOYSTICK_LF (1 << 26)


#define MASK_MIN 1
#define MASK_MAX 8
#define MASK_DEFAULT MASK_MAX
#define DELAY_MIN 50000
#define DELAY_MAX 500000
#define DELAY_DEFAULT DELAY_MAX
#define DELAY_DT 50000
#define JOY_UP (1 << 23)
#define JOY_RT (1 << 24)
#define JOY_DN (1 << 25)
#define JOY_LF (1 << 26)

#define MACRO_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MACRO_MAX(x, y) (((x) > (y)) ? (x) : (y))

static void set_bit(volatile LPC_GPIO_TypeDef *gpio, int rbit, int set)
{
    if(set) {
        gpio->FIOSET |= (1 << rbit);
        return;
    }
    
    gpio->FIOCLR |= (1 << rbit);
}

int __attribute__((noreturn)) main(void)
{
    unsigned char v;
    unsigned long t, w;
    unsigned long bit;
    unsigned short mask;
    unsigned long delay;
    uint32_t jc, jp;
    uint32_t jbc, jbp;
    
    SystemInit();

    /* Set P1.XX (joystick) for INPUT */
    LPC_GPIO1->FIODIR &= ~JOY_UP;
    LPC_GPIO1->FIODIR &= ~JOY_RT;
    LPC_GPIO1->FIODIR &= ~JOY_DN;
    LPC_GPIO1->FIODIR &= ~JOY_LF;
    
    /* Set P1.XX (LED stack) for OUTPUT */
    LPC_GPIO1->FIODIR |= (1 << 28);
    LPC_GPIO1->FIODIR |= (1 << 29);
    LPC_GPIO1->FIODIR |= (1U << 31);
    
    /* Set P2.XX (LED stack) for OUTPUT */
    LPC_GPIO2->FIODIR |= (1 << 2);
    LPC_GPIO2->FIODIR |= (1 << 3);
    LPC_GPIO2->FIODIR |= (1 << 4);
    LPC_GPIO2->FIODIR |= (1 << 5);
    LPC_GPIO2->FIODIR |= (1 << 6);
    
    v = 0;
    t = 0;
    w = 0;
    bit = 0;
    mask = MASK_DEFAULT;
    delay = DELAY_DEFAULT;
    jc = jp = 0;
    for(;; t++) {
        /* Query current joystick state */
        jc = LPC_GPIO1->FIOPIN;
        
        /* JOY_UP - grow the LED stack */
        jbc = jc & JOY_UP;
        jbp = jp & JOY_UP;
        if((jbc != jbp) && !jbc)
            mask++;
        
        /* JOY_DN - shrink the LED stack */
        jbc = jc & JOY_DN;
        jbp = jp & JOY_DN;
        if((jbc != jbp) && !jbc)
            mask--;
        
        /* JOY_LF - decrease the delay */
        jbc = jc & JOY_LF;
        jbp = jp & JOY_LF;
        if((jbc != jbp) && !jbc)
            delay -= DELAY_DT;
        
        /* JOY_RT - increase the delay */
        jbc = jc & JOY_RT;
        jbp = jp & JOY_RT;
        if((jbc != jbp) && !jbc)
            delay += DELAY_DT;
     
        /* Clamp values */
        mask = MACRO_MAX(mask, MASK_MIN);
        mask = MACRO_MIN(mask, MASK_MAX);
        delay = MACRO_MAX(delay, DELAY_MIN);
        delay = MACRO_MIN(delay, DELAY_MAX);
        
        /* Store the joystick state */
        jp = jc;
        
        if(t < w) {
            /* Tick-based delay */
            continue;
        }
        
        /* The magic bit that shifts */
        v = (unsigned char)(1 << (bit++ % mask));
        
        /* Output to P1.XX */
        set_bit(LPC_GPIO1, 28, v & (1 << 0));
        set_bit(LPC_GPIO1, 29, v & (1 << 1));
        set_bit(LPC_GPIO1, 31, v & (1 << 2));
        set_bit(LPC_GPIO2, 2, v & (1 << 3));
        set_bit(LPC_GPIO2, 3, v & (1 << 4));
        set_bit(LPC_GPIO2, 4, v & (1 << 5));
        set_bit(LPC_GPIO2, 5, v & (1 << 6));
        set_bit(LPC_GPIO2, 6, v & (1 << 7));
        
        /* Wait some time */
        w = t + delay;
    }
}
