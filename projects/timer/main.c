#include <lpc17xx.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "lib_joy.h"
#include "lib_lcd.h"
#include "lib_lcd_ibm_8x16.h"

static size_t st_millis = 0;
static bool st_counting = true;
void __attribute__((used)) SysTick_Handler(void)
{
    if(!st_counting)
        return;
    st_millis++;
}

extern uint32_t SystemFrequency;
int __attribute__((noreturn)) main(void)
{
    unsigned long t;
    size_t ms, s, m, h;
    
    SystemInit();
    joy_init();
    lcd_init();
    
    lcd_clear(0x0000);
    
    SysTick->LOAD = (SystemFrequency / 1000) - 1;
    SysTick->VAL = 0x00000000;
    SysTick->CTRL = (1 << 0) | (1 << 1) | (1 << 2);
    
    for(t = 0;; t++) {
        joy_query();
        
        if(joy_pressed(JOY_DN)) {
            lcd_clear(0x0000);
            st_millis = 0;
        }
        
        if(joy_pressed(JOY_LF))
            st_counting = !st_counting;
        
        if(t % 5000 == 0) {
            ms = st_millis % 1000;
            s = st_millis / 1000;
            m = s / 60;
            h = m / 60;
            lcd_bprintf(&IBM_8x16, 0xFFFF, 0x0000, 1, 1, "%002zu:%02zu:%02zu.%03zu", h, m % 60, s % 60, ms);
            lcd_bprintf(&IBM_8x16, st_counting ? 0x07EF : 0xFA08, 0x0000, 1, 3, st_counting ? "COUNT" : "PAUSE");
        }
        
        joy_store();
    }
}
