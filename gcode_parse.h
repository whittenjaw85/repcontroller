#ifndef _GCODE_PARSE_H
#define _GCODER_PARSE_H

#include <stdio.h>

#include "dda.h"

//decimal floating point structure
// ' a real floating point should at least have signed
// exponent. resulting value is mantissa*10^{-(exponent-1)}*((2*sign)-1)
//
typdef struct{
    uint32_t mantissa;
    uint8_t exponent :7; //scale mantissa by 10^{-exponent}
    uint8_t sign :1; //pos/neg
} decfloat;


//Receive buffer data
enum{
    cG = 1,
    cM,
    cX,
    cY,
    cZ,
    cE,
    cF,
    cS,
    cP,
    cT,
    cN,
    cCHECKSUM,
    cSEMICOMMENT,
    cPARENSCOMMENT
};
typdef struct{
    struct{
        uint8_t seen_G :1;
        uint8_t seen_M :1;
        uint8_t seen_X :1;
        uint8_t seen_Y :1;
        uint8_t seen_Z :1;
        uint8_t seen_E :1;
        uint8_t seen_F :1;
        uint8_t seen_S :1;
        uint8_t seen_P :1;
        uint8_t seen_T :1;
        uint8_t seen_N :1;
        uint8_t seen_checksum :1;
        uint8_t seen_semi_comment :1;
        uint8_t seen_parens_comment :1;

        uint8_t option_all_relative :1;
        uint8_t option_e_relative :1;
        uint8_t option_inches :1;
    };
    uint8_t G; //command number
    uint8_t M; //command number
    TARGET target; //target position x, y, z, e, & f
    uint16_t S; // s-word various uses
    uint16_t P; // p-word various uses
    uint8_t T; // t-word tool index
    uint32_t N; //line number
    uint32_t N_expected; //expected line number
    uint8_t checksum_read; //checksum in gcode command
    uint8_t checksum_calculated; //we calculate this

} GCODE_COMMAND;

//Command being processed
extern GCODE_COMMAND next_target;

void gcode_init(void);

//accept the next character and process it
void gcode_parse_char(uint8_t c);

//uses global variable next_target.N
void request_resend(void);

#endif //_GCODE_PARSE_H
