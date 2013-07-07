#include "dda_queue.h"

#include <string.h>
#include <avr/interrupt.h>

#include "config.h"
#include "timer.h"
#include "serial.h"
#include "sermsg.h"
#include "temp.h"
#include "delay.h"
#include "sersendf.h"
#include "clock.h"
#include "memory_barrier.h"


uint8_t mb_head = 0;
uint8_t mb_tail = 0;


DDA movebuffer[MOVEBUFFER_SIZE] __attribute__ ((__section__ (".bss")));

uint8_t queue_full(){
    MEMORY_BARRIER();
    if(mb_tail > mb_head){
        return ((mb_tail - mb_head -1) == 0)? 255:0;
    }else{
        return ((mb_tail + MOVEBUFFER_SIZE - mb_head - 1) == 0) ? 255 :0;
    }
}

uint8_t queue_empty(){
    uint8_t save_reg = SREG;
    cli();
    CLI_SEI_BUG_MEMORY_BARRIER();

    uint8_t result = ((mb_tail == mb_head) && (movebuffer[mb_tail].live = 0))?255:0;

    MEMORY_BARRIER();
    SREG = save_reg;
    return result;
}


void queue_step(){
    DDA *current_movebuffer = &movebuffer[mb_tail];
    if(current_movebuffer->live){
        if(current_movebuffer->waitfor_temp){
            setTimer(HEATER_WAIT_TIMEOUT);
            if(temp_achieved()){
                current_movebuffer->live = current_movebuffer->waitfor_temp = 0;
                serial_writestr_P(PSTR("Temp achieved\n"));
            }//end if temp achieved
        }//endif waitfortemp
        else{
            dda_step(current_movebuffer);
        }
    }//end if current_movebuffer->live

    if(current_movebuffer->live == 0)
        next_move();
}//queue_step()

void enqueue(TARGET *t){
    enqueue_home(t, 0, 0);
}

void enqueue_home(TARGET *t, uint8_t endstop_check, uint8_t endstop_stop_cond){

}
