#include "gcode_parse.h"

#include <string.h>
#include "serial.h"
#include "sermsg.h"
#include "dda_queue.h"
#include "debug.h"
#include "heater.h"
#include "sersendf.h"

#include "gcode_process.h"

//Current or prev gcode word
uint8_t last_field = 0;

//Crude crc
#define crc(a, b) (a^b)

//floating point data storage
decfloat read_digit __attribute__ ((__section__ (".bss")));

GCODE_COMMAND next_target __attribute__ ((__section__ (".bss")));

//dec_float_to_int is most susceptible to variable overflow
// For evaluation, we assume a build room of +/- 1000mm and STEPS_PER_MM_x between 1.000 and 4096.

//df->mantissa : +- 0..1048075 (20-bit - 500 for rounding)
//df->exponent : 0, 2, 3, 4, or 5 (10-bit)
//multiplicand: 1000 (10-bit)
//
////NO INCHES!!
#define DECFLOAT_EXP_MAX 3 // 1um is our preceision
#define DECFLOAT_MANT_MM_MAX 4289967 //4290 mm

extern const uint32_t powers[]; //defined in sermsg.c

//Convert a floating point input value into an integer with scaling
static uint32_t decfloat_to_int(decfloat *df, uint16_t multiplicand){
    uint32_t r = df->mantissa;
    uint32_t e = df->exponent;

    if(e) //decimal, but no digits, e=2 means decimal point with one digit, so it's too high by 1
        e--;

    //Increase range for mm by factor 1000 and for inches by factor 100.
    while ( (e && (multiplicand % 10)) == 0){
        multiplicand /= 10;
        e--;
    }

    r *= multiplicand;
    if(e)
        r = (r + powers[e]/2)/powers[e];

    return df->sign ? -(uint32_t)r : (int32_t)r;
}//end decfloat to int

void gcode_init(void){
    //gcc guarantees allvariables are initialized to zero
    //Assume G1 by default
    next_target.code = cG;
    next_target.G = 1;
#ifndef E_ABSOLUTE
    next_target.option_e_relative = 1;
#endif
}

void gcode_parse_char(uint8_t c)
{

}
