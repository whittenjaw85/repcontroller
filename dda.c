#include "dda.h"


#include <math.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "dda_maths.h"
//#include "timer.h"
//#include "serial.h"
//#include "sermsg.h"
//#include "gcode_parse.h"
//#include "dda_queue.h"
//#include "debug.h"
//#include "sersendf.h"
//#include "pinio.h"
#include "config.h"

//brief target position of the last move in the queue
TARGET startpoint __attribute__ ((__section__ (".bss")));

TARGET startpoint_steps __attribute__ ((__section__ (".bss")));

TARGET startpoint_position __attribute__ ((__section__ (".bss")));

MOVE_STATE move_state __attribute__ ((__section__ (".bss")));

//Initialize DDA movement structs
void dda_init(void){
    if(startpoint.F == 0)
        startpoint.F = next_target.target.F = SEARCH_FEEDRATE_Z;
}

void dda_new_startpoint(void){
    uint8_t i = 0;
    for(i=0;i<4;i++) startpoint_steps[i] = um_to_steps(startpoint[i],i);
}



//Create a DDA given current position and target position
void dda_create(DDA *dda, TARGET *target)
{
    uint32_t steps;
    uint32_t delta[AXES_QUANTITY]; //x, y, z, e
    uint32_t distance, c_limit, c_limit_calc;

    //Initialize to a known state
    dda->allflags = 0;

    //if(DEBUG_DDA && (debug_flags & DEBUG_DDA))
    //    serial_writestr_P(P

    //End at passed target
    memcpy(&(dda->endpoint), target, sizeof(TARGET));

    //loop axes
    dda->dir = 0; //clear directional flag
    uint8_t i = 0;
    //X,Y,Z,E calculations
    for(i=0;i<4;i++)
    {
        //Calculate the total change
        delta[i] = (uint32_t)labs(target.pos[i] - startpoint.pos[i]);

        //Calculate number of steps to target
        steps = um_to_steps(target.pos[i], i);

        //Calculate the startpoint steps
        dda->delta[i] = labs(steps - startpoint_steps[i]);

        startpoint_steps[i] = steps;

        //Calculate direction to target
        dda->dir[i] |= (target.pos[i] >= startpoint.pos[i])?(1 << i):0;
    }//end for axis loop


    //DDA settings

    //Get Baseline
    dda->total_steps = 0;
    for(i=0;i<4;i++)
    {
        if(dda->total_steps < dda->delta[i])
            dda->total_steps = dda->delta[i];
    }

    //If there is nowhere to go... NOP
    if(dda->total_steps == 0)
        dda->nullmove = 1;
    else{
        power_on();
        stepper_enable();
        x_enable();
        y_enable();
        e_enable();
    }

    //Hopeful approximation
    if(delta[2] == 0)//z movement is zero
        distance = approx_distance(delta[0], delta[1]);
    else if(delta[0] == 0 && delta[1] == 0)
        distance = approx_distance(delta[2]);
    else
        distance = approx_distance_3(delta);

    if(distance < 2)
        distance = delta[4];

    uint32_t move_duration = ((distance*2400)/dda->total_steps)*(F_CPU/4000);


    //Max speed calculation
    c_limit = 0;
    for(i=0;i<4;i++)
    {
        c_limit_calc = ((delta[i]*2400L)/dda->total_steps*(F_CPU/40000)/MAXIMUM_FEEDRATE_X) << 8;
        if(c_limit_calc > c_limit)
            c_limit = c_limit_calc;
    }//end for

    dda->c = (move_duration / target->F) << 8;
    if(dda->c < c_limit)
        dda->c = c_limit;

    memcpy(&startpoint, target, sizeof(TARGET));
}


void dda_start(DDA * dda)
{
    //Called from interrupt, so necessarily short
    if(!dda->nullmove)
    {
        psu_timeout = 0;
        if(dda->delta[2])//z
            z_enable();

        //set direction outputs
        uint8_t i = 0;
        for(i=0;i<4;i++)
        {
            uint8_t dir = (((1<<i)&dda->dir) >> i)
            direction(dir, i);
        }


        //Initialize state variables
        move_state.counter[0] = move_state.counter[1] = move_state.counter[2] = \
                                move_state.counter[3] = -(dda->total_steps>>1);
        memcpy(&move_state.steps[0], &dda->delta[0], sizeof(uint32_t)*4);

        //Ensure that the dda starts
        dda->live = 1;

        //set timeout for first step
        setTimer(dda->c >> 8);
    }//end if
    else{
        current_position.F = dda->endpoint.F;
    }//end else
}
