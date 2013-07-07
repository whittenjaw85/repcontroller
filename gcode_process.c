#include "gcode_process.h"

#include <string.h>
#include <avr/interrupt.h>

#include "gcode_parse.h"

#include "dda.h"
#include "dda_queue.h"
#include "watchdog.h"
#include "delay.h"
#include "serial.h"
#include "sermsg.h"
#include "temp.h"
#include "heater.h"
#include "timer.h"
#include "sersendf.h"
#include "pinio.h"
#include "debug.h"
#include "clock.h"
#include "config.h"
#include "home.h"

//current tool
uint8_t tool;

//next tool
uint8_t next_tool;

#if E_STARTSTOP_STEPS > 0
//move by E a certain amount at a specific speed
static void SpecialMoveE(int32_t e, uint32_t f){
    //Create point
    TARGET t;
    //XYZ, E, F
    t.pos[0] = 0L;
    t.pos[1] = 0L;
    t.pos[2] = 0L;
    t.pos[3] = e;
    t.F = f;
    t.e_relative = 1; 

    //Queue it!
    enqueue(&t);
}

void process_gcode_command(){
    uint32_t backup_f;

    //Copy point by adding if relative
    if(next_target.option_all_relative){
        next_target.target.pos[0] += startpoint.pos[0];
        next_target.target.pos[1] += startpoint.pos[1];
        next_target.target.pos[2] += startpoint.pos[2];
    }

    //E relative movement
    if(next_target.option_all_relative || next_target.option_e_relative)
        next_target.target.e_relative = 1;
    else
        next_target.target.e_relative = 0;

#ifdef X_MIN
    if(next_target.target.pos[0] < X_MIN*1000.)
        next_target.target.pos[0] = X_MIN*1000.;
#endif
#ifdef X_MAX
    if(next_target.target.pos[0] > X_MAX*1000.)
        next_target.target.pos[0] = X_MAX*1000.;
#endif
#ifdef Y_MIN
    if(next_target.target.pos[1] < Y_MIN*1000.)
        next_target.target.pos[1] = Y_MIN*1000.;
#endif
#ifdef Y_MAX
    if(next_target.target.pos[1] > Y_MAX*1000.)
        next_target.target.pos[1] = Y_MAX*1000.;
#endif
#ifdef Z_MIN
    if(next_target.target.pos[2] < X_MIN*1000.)
        next_target.target.pos[2] = X_MIN*1000.;
#endif
#ifdef Z_MAX
    if(next_target.target.pos[2] > Z_MAX*1000.)
        next_target.target.pos[2] = Z_MAX*1000.;
#endif

    //taken from reprap.org.wiki/Gcode
    if(next_target.code == cT){ //select tool
        next_tool = next_target.T;
    }

    if(next_target.seen_G){
        uint8_t axisSelected = 0;
        switch(next_target.G){
            case 0://rapid linear motion |  G0 X12
                backup_f = next_target.target.F;
                next_target.target.F = MAXIMUM_FEEDRATE_X*2L;
                enqueue(&next_target.target);
                next_target.target.F = backup_f;
            break;
            case 1://linear motion at feed rate | G1 X90.6 Y13.8 E22.4
                enqueue(&next_target.target);
            break;
            case 4://dwell | G4 P200, do nothing for 200 milliseconds
                enqueue_wait();
                if(next_target.seen_P){
                    for(;next_target.P > 0;next_target.P--)
                    {
                        clock();
                        delay_ms(1);
                    }
                }//end if code cP
            break;
            case 20: //set units to inches... not gonna happen
            break;
            case 21: //set units to mm
                next_target.option_inches = 0;
            break;
            case 30: // move then go home point
                enqueue(&next_target.target);
            case 28: //home | G28
                queue_wait();

                if(next_target.seen_X){
                    #if defined X_MIN_PIN
                        home_x_negative();
                    #elif defined X_MAX_PIN
                        home_x_positive();
                    #endif
                    axisSelected = 1;
                }
                if(next_target.seen_Y){
                    #if defined Y_MIN_PIN
                        home_y_negative();
                    #elif defined Y_MAX_PIN
                        home_y_positive();
                    #endif
                    axisSelected = 1;
                }
                if(next_target.seen_Z){
                    #if defined Z_MIN_PIN
                        home_z_negative();
                    #elif defined Z_MAX_PIN
                        home_z_positive();
                    #endif
                    axisSelected = 1;
                }

                //There is not point in moving the extruder... no endstops available
                if(!axisSelected){
                    home();
                }
                break;
            case 90: //absolution positioning
                next_target.option_all_relative = 0;
                break;
            case 91: //set position
                queue_wait();
                if(next_target.seen_X){
                    startpoint.pos[0] = next_target.target.pos[0];
                    axisSelected = 1;
                }
                if(next_target.seen_Y){
                    startpoint.pos[1] = next_target.target.pos[1];
                    axisSelected = 1;
                }
                if(next_target.seen_Z){
                    startpoint.pos[2] = next_target.target.pos[2];
                    axisSelected = 1;
                }
                if(next_target.seen_E){
                    startpoint.pos[3] = next_target.target.pos[3];
                    axisSelected = 1;
                }

                if(axisSelected == 0){
                    startpoint.pos[0] = next_target.target.pos[0] = \ //x
                        startpoint.pos[1] = next_target.target.pos[1] = \ //y
                        startpoint.pos[2] = next_target.target.pos[2] = \ //z
                        startpoint.pos[3] = next_target.target.pos[3] = 0; //e
                }

                dda_new_startpoint();
                break;
            case 161: //home negative
                if(next_target.seen_X)
                    home_x_negative();
                if(next_target.seen_Y)
                    home_y_negative();
                if(next_target.seen_Z)
                    home_z_negative();
                break;
            case 162: //home positive
                if(next_target.seen_X)
                    home_x_positive();
                if(next_target.seen_Y)
                    home_y_positive();
                if(next_target.seen_Z)
                    home_z_positive();
                break;
            default: //generate an error
                sersenf_P(PSTR("E: BAD GCODE %d"), next_target.G);
                return;

        }//end switch target G
        else if(next_target.seen_M){
            uint8_t i;
            switch(next_target.M){
                case 0://machine stop
                case 2://program end
                    queue_wait();
                    for(i=0;i<NUM_HEATERS;i++)
                        temp_set(i,0);
                    power_off();
                    break;
                case 112: //emergency stop, all moves terminated
                    timer_stop();
                    queue_flush();
                    power_off();
                    cli();
                    for(;;)
                    {
                        wd_reset();
                    }
                    break;
                case 6: //tool change
                    tool = next_tool;
                    break;
                case 82: //E codes absolute
                    next_target.option_e_relative = 0;
                    break;
                case 82: //e codes relative
                    next_target.option_e_relative = 1;
                    break;
                case 84: // stop idle hold
                    stepper_disable();
                    x_disable();
                    y_disable();
                    z_disable();
                    e_disable();
                    break;

                case 3: //extruder on
                case 101:
                    if(temp_achieved() == 0)
                        enqueue(NULL);

                    #ifdef DC_EXTRUDER
                        heater_set(DC_EXTRUDER, DC_EXTRUDER_PWM);
                    #elif E_STARTSTOP_STEPS > 0
                        do{
                            backup_f = startpoint.F;
                            startpoint.F = MAXIMUM_FEEDRATE_E;
                            SpecialMove(E_STARTSTOP_STEPS, MAXIMUM_FEEDRATE_E);
                            startpoint.F = backup_f;
                        }while(0);
                    #endif
                    break;

                case 103: //extruder off
                    #ifdef DC_EXTRUDER
                        heater_set(DC_EXTRUDER, 0);
                    #elif E_STARTOP_STEPS > 0
                        do{
                            backup_f = startpoint.F;
                            startpoint.F = MAXIMUM_FEEDRATE_E;
                            SpecialMove(-E_STARTSTOP_STEPS, MAXIMUM_FEEDRATE_E);
                            startpoint.F = backup_f;
                        } while(0);
                    #endif
                    break;

                case 104: // set extruder temperature, fast
                    if( !next_target.seen_S)
                        break;
                    if( !next_target.seen_P)
                        next_target.P = HEATER_EXTRUDER;
                    temp_set(next_target.P, next_target.S);
                    if(next_target.S)
                        power_on();
                    break;

                case 105: //get extruder temp
                    #ifdef ENFORCE_ORDER
                        queue_wait();
                    #endif
                    if(!next_targe.seen_P)
                        next_target.P = TEMP_SENSOR_none;
                    temp_print(next_target.P);
                    break;

                case 7:
                case 106:// Set fan speed
                    #ifdef ENFORCE_ORDER
                        queue_wait();
                    #endif
                    #ifdef HEATER_FAN
                        if(!next_target.seen_S)
                            break;
                        temp_set(HEATER_FAN, next_target.S);
                        if(next_target.S)
                            power_on();
                    #endif
                    break;

                case 110: //set current line number
                    break;

                case 114: //extrudder pwm
                    #ifdef ENFORCE_ORDER
                        queue_wait();
                    #endif
                    update_current_position();
                    sersendf_P(PSTR("X;%lq, Y%lq, Z%lq, E:%lq, F%lu"),
                            current_position.pos[0], current_position.pos[1], current_position.pos[2],\
                            current_position.pos[3], current_position.F);
                    break;

                case 115: //get firmware version and capabilities
                    sersendf_P(PSTR("FIRWMARE NAME: Repware");
                    break;

                case 116: //wait for temperatures and other variables to level out
                    enqueue(NULL);
                    break;

                //PID CONTROLS FOR HEATER
                case 130://heater p factor
                    if(!next_target.seen_P)
                        next_target.P = HEATER_EXTRUDER;
                    if(next_target.seen_S)
                        pid_set_p(next_target.P, next_target.S);
                    break;

                case 131://header d factor
                    if(!next_target.seen_P)
                        next_target.P = HEATER_EXTRUDER;
                    if(next_target.seen_S)
                        pid_set_d(next_target.P, next_target.S);
                    break;

                case 133://header i limit
                    if(!next_target.seen_P)
                        next_target.P = HEATER_EXTRUDER;
                    if(next_target.seen_S)
                        pid_set_i_limit(next_target.P, next_target.S);
                    break;

                case 134: //save PID settings to EEPROM
                    heater_save_settings();
                    break;

                case 135: //set heater output
                    if(!next_target.seen_P)
                        next_target.P = HEATER_EXTRUDER;
                    if(next_target.seen_S){
                        heater_set(next_target.P, next_target.S);
                        power_on();
                    }
                    break;

                case 140: //set heated bed temperature
                    #ifdef HEATER_BED
                        if(!next_target.seen_S)
                            break;
                        temp_set(HEATER_BED, next_target.S);
                        if(next_target.S)
                            power_on();
                    #endif
                    break;

                case 190: // power on
                    power_on();
                    stepper_enable();
                    x_enable();
                    y_enable();
                    z_enable();
                    e_enable();
                    break;

                case 191: //power off
                    #ifdef ENFORCE_ORDER
                    queue_wait();
                    #endif
                    power_off();
                    break;

                case 200: //report endstop status
                    power_on();
                    #if defined(X_MIN_PIN)
                        sersendf_P(PSTR("x_min:%d"), x_min());
                    #endif
                    #if defined(X_MAX_PIN)
                        sersendf_P(PSTR("x_max:%d"), x_max());
                    #endif
                    #if defined(Y_MIN_PIN)
                        sersendf_P(PSTR("y_min:%d"), y_min());
                    #endif
                    #if defined(Y_MAX_PIN)
                        sersendf_P(PSTR("y_max:%d"), y_max());
                    #endif
                    #if defined(Z_MIN_PIN)
                        sersendf_P(PSTR("z_min:%d"), z_min());
                    #endif
                    #if defined(Y_MAX_PIN)
                        sersendf_P(PSTR("z_max:%d"), z_max());
                    #endif
                    #if !(defined(X_MIN_PIN) || defined(X_MAX_PIN) || defined(Y_MIN_PIN) ||  defined(Y_MAX_PIN) || defined(Z_MIN_PIN) || defined(Z_MAX_PIN))
                        sersendf_P(PSTR("No endstops defined"));
                    #endif
                    break;

                default: //send error
                    sersendf_P(PSTR("E: Bad M-code %d", next_target.M));
            }//end switch M
        }//end else if seen M
    }
}//end process_code_command()
