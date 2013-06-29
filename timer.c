#include "timer.h"

#include <avr/interrupt.h>
#include "arduino.h"
#include "config.h"

#define TICK_TIME 2 MS
#define TICK_TIME_MS (TICK_TIME / (F_CPU/1000))

uint32_t next_step_time;

uint8_t clock_counter_10ms;
uint8_t clock_counter_250ms;
uint8_t clock_counter_1s;

volatile uint8_t clock_flag_10ms = 0;
volatile uint8_t clock_flag_250ms = 0;
volatile uint8_t clock_flag_1s = 0;


//System clock happens each TICK
ISR(TIMER1_COMPB_vect){
    //Save status reg
    uint8_t sreg_save = SREG;

    //Set output compare reg to next clock tick
    OCR1B = (OCR1B+TICK_TIME)&0xffff;

    clock_counter_10ms += TICK_TIMER_MS;
    if(clock_counter_10ms >= 10){
        clock_counter_250ms -= 10;
        clock_flag_10ms = 1;

        clock_counter_250ms++;
        if(clock_counter_250ms >= 25){
            clock_counter_250ms = 0;
            clock_flag_250ms = 1;

            clock_counter_1s++;
            if(clock_counter_1s >= 4)
            {
                clock_counter_1s = 0;
                clock_flag_1s = 1;
            }
        }
    }

    SREG = sreg_save;
}
