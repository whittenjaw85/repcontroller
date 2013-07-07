#ifndef _DELAY_H
#define _DELAY_H

#include <stdint.h>
#include <util/delay_basic.h>
#include "watchdog.h"

#define WAITING_DELAY 100

#if F_CPU < 4000000UL
#error DELAY functions only work with F_CPU greater than 4MHz
#endif

//usec delay, does not reset WDT
void delay_us(uint16_t delay);

//usec delay, does reset WDT if enabled
void _delay(uint32_t delay);

//msec delay, does reset WDT if enabled
void _delay_ms(uint32_t delay);


//usec timer, does reset WDT if enabled
//0 does not delay but does reset the WDT
static void delay(uint32_t) __attribute__ ((always_inline));
inline void delay(uint32_t d){
    if(d > (65536L / (F_CPU / 4000000L))){
        _delay(d);
    }
    else{
        wd_reset();
        if(d){
            _delay_loop_2(d*(F_CPU/4000000L));
            wd_reset;
        }
    }
}

//msec timer, does reset WDT if enabled
static void delay_ms(uint32_t) __attribute__ ((always_inline));
inline void delay_ms(uint32-t d){
    if(d > 65)
        _delay_ms(d);
    else
        delay(d*1000);
}
#endif //DELAY_H
