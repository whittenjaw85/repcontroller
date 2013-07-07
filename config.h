/* This file will configure the rest of the interface */

/*
1. Hardware
2. Acceleration
3. Pinout
4. Temperature
5. Heater
6. Communication Settings
7. Miscellaneous
8. PWM Mapping


*/
/*
                  0. Important global variables


                  TARGET startpoint | point used for calculation
                  TARGET current_position | used during gcode interpretation
                  GCODE_COMMAND next_target | used to control the gcode parser
                  uint8_t last_field | last character received


*/
//                 1. Hardware
//

#include "arduino.h"
#ifndef F_CPU
    #define F_CPU 16000000UL //16 MHz
#endif

//Motherboard
//#define HOST

//Number of axes
#define AXES_QUANTITY 4
#define GANTRY 3

//Belt is T2.5, 400 steps per rotation
#define STEPS_PER_M_X 320000
#define STEPS_PER_M_Y 320000

//Threaded rod is M8x1.25
#define STEPS_PER_M_Z 320000

//Extruder Rate
#define STEPS_PER_M_E 320000


//Feedrates
#define MAXIMUM_FEEDRATE_X 200
#define MAXIMUM_FEEDRATE_Y 200
#define MAXIMUM_FEEDRATE_Z 200
#define SEARCH_FEEDRATE_X 50
#define SEARCH_FEEDRATE_Y 50
#define SEARCH_FEEDRATE_Z 50


///
///		2. Acceleration
///

//Filament return steps (when stops extruding)
#define E_STARTSTOP_STEPS   20

#define ACCELERATION_RAMPING

#define ACCELERATION 1000.

///
///		3. Pinouts
///


#define X_STEP_PIN	//PB4
#define X_DIR_PIN	//PC0
#define X_EN_PIN	//PB0

#define Y_STEP_PIN	//PD0
#define Y_DIR_PIN	//PD2
#define Y_EN_PIN	//PD1

#define Z_STEP_PIN	//PC4
#define Z_DIR_PIN	//PC6
#define Z_EN_PIN	//PC5

#define E_STEP_PIN	//PB7
#define E_DIR_PIN	//PC2
#define E_EN_PIN	//PC1

#define BED_EN_PIN	//PE4
#define BED_TEMP_PIN	//PF0

#define HOTEND_EN_PIN	//PD3
#define HOTEND_TEMP_PIN //PF1

#define FAN_EN_PIN	//PD4




///
///		4. Temperature Sensors
///




#define TEMP_HYSTERESIS

#define TEMP_RESIDENCY_TIME	60
#define TMP_THERMISTOR





///
///		5. Heaters
///




/* Not sure what to put here */





///
///		6. Communication Settings
///


#define BAUD	    115200 //fast as possible

//XONXOFF needed for Gcode from raw-data terminal
#define XONXOFF






///
///		7. Miscellaneous Options
///









