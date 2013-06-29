#ifndef _TIMER_H
#define _TIMER_H

#include <stdint.h>
#include <avr/io.h>

#define US * (F_CPU/1000000L)
#define MS * (F_CPU/1000)

extern volatile uint8_t clock_flag_10ms;
extern volatile uint8_t clock_flag_250ms;
exterm volatile uint8_t clock_flag_1s;

//if the bit is set execute the block once then clear flag
#define ifclock(F) for(;F;F=0)

void timer_init(void) __attribute__ ((cold));
void setTimer(uint32_t delay);
void timer_stop(void);

#endif //_TIMER_H
