#ifndef _DDA_H
#define _DDA_H

#include <stdint.h>
#include "config.h"

//Ramping acceleration later

//0 - X
//1 - Y
//2 - Z
//3 - E
//4 - F

typedef struct{
    int32_t pos[AXES_QUANTITY];
    uint32_t F;
} TARGET;


typedef struct{
    //bresenham counters
    uint32_t counter[AXES_QUANTITY];

    //steps counter
    uint32_t steps[AXES_QUANTITY];

    //endstop debouncing
    uint8_t debounce_countmax[GANTRY]; //x,y,z
    uint8_t debounce_countmin[GANTRY];

} MOVE_STATE;


//DDA structure
typedef struct{
    TARGET endpoint;

    union{
        struct{
            //status fields
            uint8_t nullmove :1;
            uint8_t live :1;
            uint8_t waitfor_temp :1;
            uint8_t dir; //x, y, z, e (0, 1, 2, 3)
        };//end struct

        uint8_t allflags;//clearing all flags
    };//end union

    uint32_t delta[AXES_QUANTITY];//number of steps on axis
    uint32_t total_steps;
    uint32_t c;//time until next step 24.8 fixed point

    //endstop homing
    uint8_t endstop_check;
    uint8_t endstop_stop_cond;
} DDA; //end struct


//Variables
extern TARGET startpoint;
extern TARGET startpoint_steps;
extern TARGET current_position;

//Initialize DDA struct
void dda_init(void);

//Distribute a new startpoint
void dda_new_startpoint(void);

//create DDA
void dda_create(DDA *dda, TARGET *target);

//start a created DDA
void dda_start(DDA *dda) __attribute__ ((hot));

//DDA takes one step
void dda_step(DDA *dda) __attribute__ ((hot));

//Update current position
void update_current_position(void);

#endif //_DDA_H
