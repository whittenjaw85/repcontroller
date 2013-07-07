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
    next_target.seen_G = 1;
    next_target.G = 1;
#ifndef E_ABSOLUTE
    next_target.option_e_relative = 1;
#endif
}

void gcode_parse_char(uint8_t c)
{
    uint8_t checksum_char = c;

    //uppercase
    if(c > 'a' && c <= 'z')
        c &= ~32;

    //Process previous fieled
    if(last_field){
        //determine if input is a new field or EOL
        //All non-expected characters begin a new field
        if((c >= 'A' && c <= 'Z') || (c == '*') || (c == 10) || (c == 13)){
            switch(last_field){
                case 'G':
                    next_target.G = read_digit.mantissa;
                    break;
                case 'M':
                    next_target.M = read_digit.mantissa;
                    break;
                case 'X':
                    next_target.target.pos[0] = decfloat_to_int(&read_digit, 1000);//metric only
                    break;
                case 'Y':
                    next_target.target.pos[1] = decfloat_to_int(&read_digit, 1000);
                    break;
                case 'Z':
                    next_target.target.pos[2] = decfloat_to_int(&read_digit, 1000);
                    break;
                case 'E':
                    next_target.target.pos[3] = decfloat_to_int(&read_digit, 1000);
                    break;
                case 'F':
                    next_target.target.F = decfloat_to_int(&read_digit, 1000);
                    break;
                case 'S': //temperature
                    //perform temperature control operations, not normally here, but it generates less code
                    if( (next_target.M == 104) || (next_target.M == 109) || (next_target.M == 140))
                        next_target.S = decfloat_to_int(&read_digit, 4);
                    else if((next_target.M >= 130) && (next_target.M <= 132))
                        next_target.S = decfloat_to_int(&read_digit, PID_SCALE);
                    else
                        next_target.S = decfloat_to_int(&read_digit, 1);
                    break;
                case 'P':
                    next_target.P = decfloat_to_int(&read_digit, 1);
                    break;
                case 'T':
                    next_target.T = read_digit.mantissa;
                    break;
                case 'N':
                    next_target.N = decfloat_to_int(&read_digit, 1);
                    break;
                case '*':
                    next_target.checksum_read = decfloat_to_int(&read_digit, 1);
                    break;
            }//end switch last field

            //reset for the next field
            last_field = 0;'
            read_digit.sign = read_digit.mantissa = read_digit.exponent = 0;

        }//end if character outside expected range

        //skip comments
        if((next_target.seen_semi_comment == 0) && (next_target.seen_parens_comment == 0)){
            if((c >= 'A' && c <= 'Z') || c == '*'){
                last_field = c;
            }

            //process character
            switch(c){
                case 'G':
                    next_target.seen_G = 1;
                    next_target.seen_M = 0;
                    break;
                case 'M':
                    next_target.seen_G = 0;
                    next_target.seen_M = 1;
                    break;
                case 'X':
                    next_target.seen_X = 1;
                    break;
                case 'Y':
                    next_target.seen_Y= 1;
                    break;
                case 'Z':
                    next_target.seen_Z= 1;
                    break;
                case 'E':
                    next_target.seen_E= 1;
                    break;
                case 'F':
                    next_target.seen_F= 1;
                    break;
                case 'S':
                    next_target.seen_S= 1;
                    break;
                case 'P':
                    next_target.seen_P= 1;
                    break;
                case 'T':
                    next_target.seen_T= 1;
                    break;
                case 'N':
                    next_target.seen_N= 1;
                    break;
                case '*':
                    next_target.seen_checksum = 1;
                    break;

                //comments
                case ';':
                    next_target.seen_semi_command= 1;
                    break;
                case '(':
                    next_target.seen_parens_comment = 1;
                    break;

                //number formatting
                case '-':
                    read_digit.sign = 1;
                    read_digit.exponent = 0;
                    read_digit.mantissa = 0;
                    break;
                case '.':
                    if(digit_read.exponent == 0)
                        read_digit.exponent = 1;
                    break;

                default: //digit processing
                    if( (c >= '0' && c <= '9')){
                        if(read_digit.exponent < DECFLOAT_EXP_MAX + 1 &&
                                read_digit.mantissa < DECFLOAT_MANT_MM_MAX){
                            //mantissa = (mantissa*10) + atoi(c)
                            read_digit.mantissa = (read_digit.mantissa << 3) + (read_digit.mantissa << 1) + (c - '0');
                            if(read_digit.exponent)
                                read_digit.exponent++;
                        }//end if inside bounds of number capacity

                    }//end if a number
            }//end switch char
        }//end if not a comment
        else if( (next_target.seen_parens_comment == 1) && (c == ')'))
            next_target.code = 0;

        if(next_target.seen_checksum == 1)
            next_target.checksum_calculated = crc(next_target.checksum_calculated, checksum_char);

        //EOL
        if( (c == 10) || (c == 13)){
            //process
            process_gcode_command();

            //reset variables
            next_target.seen_G = next_target.seen_M = next_target.seen_X = \
                next_target.seen_Y = next_target.seen_Z = next_target.seen_E = next_target.seen_N = \
                next_target.seen_F = next_target.seen_P = next_target.seen_S = next_target.seen_T = \
                next_target.seen_checksum = next_target.seen_semi_comment = next_target.seen_parens_comment = \
                next_target.checksum_read = next_target.checksum_calculated = 0;

            //Assume G1 default
            next_target.seen_G = 1;
            next_target.G = 1;

            //Prepare memory for relative computation
            if(next_target.option_all_relative){
                next_target.target.X = next_target.target.Y = next_target.target.Z = 0;
            }
            if(next_target.option_all_relative || next_target.option_e_relative){
                next_target.target.E = 0;
            }
        }
    }//end if last-field
}//end gcode_parse_char

//Request a resend of the current line, requires global variable next_target.N valid
void request_resend(void){
    serial_writestr_P(PSTR("rs"));
    serwrite_uint8(next_target.N);
    serial_writechar('\n');
}
