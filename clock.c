#include "clock.h"


#include "pinio.h"
#include "sersendf.h"
#include "dda_queue.h"
#include "watchdog.h"
#include "temp.h"
#include "timer.h"
#include "debug.h"
#include "heater.h"
#include "serial.h"
#ifdef TEMP_INTERCOM
    #include "intercom.h"
#endif
#include "memory_barrier.h"

//Perform tasks every 0.25 seconds
static void clock_250ms(void){
    #ifndef NO_AUTO_IDLE
    if(temp_all_zero()){
        if(psu_timeout > (30*4)){
            power_off();
        }
    }else{
        uint8_t save_reg = SREG;
        cli();
        CLI_SEI_BUG_MEMORY_BARRIER();
        psu_timeout++;
        MEMORY_BARRIER();
        SREG = save_reg;
    }
    #endif

    ifclock(clock_flag_is){
        if(DEBUG_POSITION && (debug_flags & DEBUG_POSITION)){
            //current position
        }
    }//end ifclock
}//end clock250ms()

static void clock_10ms(void){
    wd_reset();
    temp_tick();
    ifclock(clock_flag_250ms){
        clock_250ms();
    }
}//clock_10ms()
