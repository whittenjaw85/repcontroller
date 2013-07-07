#include "delay.h"

//Interruptible usec delay
void delay_us(uint16_t delay)
{
    while(delay > (65536UL / (F_CPU/4000000L))){
        _delay_loop_2(65534);
        delay -= (65536L / (F_CPU/4000000L));
    }
    _delay_loop_2(delay*(F_CPU/4000000L));
}


void _delay(uint32_t delay)
{
    wd_reset();
    while(delay > (65536L / (F_CPU/4000000L))){
        _delay_loop_2(65534); //compensates for looping, so not 65536
        delay -= (65536L / (F_CPU/4000000L));
        wd_reset();j
    }
    _delay_loop_2(delay* (F_CPU/4000000L));
    wd_reset;
}

void _delay_ms(uint32_t delay){
    wd_reset();
    while(delay > 65){
        delay_us(64999);
        delay -= 65;
        wd_reset;
    }
    delay_us(delay*1000);
    wd_reset();
}
