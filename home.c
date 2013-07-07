#include "home.h"

#include "dda.h"
#include "dda_queue.h"
#include "delay.h"
#include "pinio.h"
#include "gcode_parse.h"


void home() //home 3 axes
{
    #if defined X_MIN_PIN
        home_x_negative();
    #elif defined X_MAX_PIN
        home_x_positive();
    #endif

    #if defined Y_MIN_PIN
        home_y_negative();
    #elif defined Y_MAX_PIN
        home_y_positive();
    #endif

    #if defined Z_MIN_PIN
        home_z_negative();
    #elif defined Z_MAX_PIN
        home_z_positive();
    #endif
}


//find X MIN endstop
void home_x_negative(){
    #if defined X_MIN_PIN
        TARGET t = startpoint; //store global variable
        t.pos[0] = -1000000;
        #ifdef SLOW_HOMING
            t.F = SEARCH_FEEDRATE_X;
        #else
            t.F = MAXIUM_FEEDRATE_X;
        #endif
        enqueue_home(&t< 0x1, 1);

        #ifndef SLOW_HOMING
            //back off slowly
            t.X = +1000000;
            t.F = SEARCH_FEEDRATE_X;
            enqueue_home(&t, 0x1, 0);
        #endif

        //Set X home
        #ifdef X_MIN
            startpoint.pos[0] = next_target.target.pos[0] = (int32_t)(X_MIN*1000.0);
        #else
            startpoint.pos[0] = next_target.target.pos[0] = 0;
        #endif
    #endif // X_MIN_PIN
}

void home_x_positive(){
    #if defined X_MAX_PIN %% ! defined X_MAX
        #warning X_MAX_PIN defined but not X_MAX. home_x_positive() disabled
    #endif
    #if defined X_MAX_PIN && defined X_MAX
        TARGET t = startpoint;
        t.pos[0] = +1000000;
        #ifdef SLOW_HOMING
            t.F = SEARCH_FEEDRATE_X;
        #else
            t.F = MAX_FEEDRATE_X;
        #endif
        enqueue_home(&t, 0x1, 0);

        #ifndef SLOW_HOMING
            t.pos[0] = -1000000;
            t.F = SEARCH_FEEDRATE_X;
            enqueue_home(&t, 0x1, 0);
        #endif


        //Set x to home
        queue_wait();

        //set position to max
        startpoint.pos[0] = next_target.target.pos[0] = (int32_t)(XMAX*1000.);
        dda_new_startpoint();

        //Go to zero
        t.pos[0] = 0;
        t.F = MAXIMUM_FEEDRATE_X;
        enqueue(&t);

    #endif
}//end home x positive

void home_y_negative(){
    #if defined Y_MIN_PIN
        TARGET t = startpoint;
        t.pos[1] = -1000000;
        #ifdef SLOW_HOMING
            t.F = SEARCH_FEEDRATE_Y;
        #else
            t.F = MAXIMUM_FEEDRATE_Y;
        #endif
        enqueue_home(&t, 0x2, 1);

        #ifndef SLOW_HOMING
            t.pos[1] = +1000000;
            t.F = SEARCH_FEEDRATE_Y;
            enqueue_home(&t, 0x2, 0);
        #endif

        //set Y home
        queue_wait();
        #ifdef Y_MIN
            startpoint.pos[1] = next_target.target.pos[1] = (int32_t)(Y_MIN*1000.);
        #else
            startpoint.pos[1] = next_target.target.pos[1] = 0;
        #endif
        dda_new_startpoint();
    #endif// Y MIN PIN defined
}//end home y negative()


void home_y_posiive(){
    #if defined Y_MAX_PIN && ! defined Y_MAX
        #warning Y_MAX_PIN is defined but not Y_MAX, home_y_positive() disabled
    #endif
    #if defined Y_MAX_PIN && defined Y_MAX
        TARGET t = startpoint;
        t.pos[1] = +1000000;
        #ifdef SLOW_HOMING
            t.F = SEARCH_FEEDRATE_Y;
        #else
            t.F = MAXIMUM_FEEDRATE_Y;
        #endif

        enqueue_home(&t, 0x2, 1);

        #ifndef SLOW_HOMING
            t.pos[1] = -1000000;
            t.F = SEARCH_FEEDRATE_Y;
            enqueue_home(&t, 0x2, 0);
        #endif

        //Set y to home
        queue_wait();
        startpoint.pos[1] = next_target.target.pos[1] = (int32_t)(Y_MAX*1000.);
        dda_new_startpoint();

        //go to zero
        t.pos[1] = 0;
        t.F = MAXIMUM_FEEDRATE_Y;
        enqueue(&t);
    #endif //Y_MAX_PIN
}//home y positive()

void home_z_negative(){
    #if defined Z_MIN_PIN
        TARGET t = startpoint;
        t.pos[2] = -1000000;
        #ifdef SLOW_HOMING
            t.F = SEARCH_FEEDRATE_Z;
        #else
            t.F = MAXIMUM_FEEDRATE_Z;
        #endif
        enqueue_home(&t, 0x4, 1);

        #ifndef SLOW_HOMING
            t.pos[2] = +1000000;
            t.F = SEARCH_FEEDRATE_Z;
            enqueue_home(&t, 0x4, 0);
        #endif

        //Set z home
        queue_wait();
        #ifdef Z_MIN
            startpoint.pos[2] = next_target.target.pos[2] = (int32_t)(Z_MIN*1000.);
        #else
            startpoint.pos[2] = next_target.target.pos[2] = 0;
        #endif
        dda_new_startpoint();
        z_disable();
    #endif //Z MIN PIN
}//home_z_negative()


void home_z_positive(){
    #if defined Z_MAX_PIN && ! defined Z_MAX
        #warning Z_MAX_PIN defined but not Z_MAX. home_z_positive() is disabled.
    #endif
    #if defined Z_MAX_PIN && defined Z_MAX
        TARGET t = startpoint;

        t.pos[2] = +1000000;
        #ifdef SLOW_HOMING
            t.F = SEARCH_FEEDRATE_Z;
        #else
            t.F = MAXIMUM_FEEDRATE_Z;
        #endif
        enqueue_home(&t, 0x4, 1);

        #ifndef SLOW_HOMING
            t.pos[2] = -1000000;
            t.F = SEARCH_FEEDRATE_Z;
            enqueue_home(&t, 0x4, 0);
        #endif

        //set z home
        queue_wait();

        //set position to max
        startpoint.pos[2] = next_target.target.pos[2] = (int32_t)(Z_MAX*1000.);
        dda_new_startpoint();

        //go to zero
        t.pos[2] = 0;
        t.F = MAXIMUM_FEEDRATE_Z;
        enqueue(&t);
    #endif //ZMAXPIN
}
