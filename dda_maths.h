#ifndef _DDA_MATHS
#define _DDA_MATHS

#include <stdint.h>
#include "config.h"

//Return rounded result with multiplicand*multiplier/divisor
const int32_t muldivQR(int32_t muliplicand, uint32_t qn, uint32_t rn, uint32_t divisor);

//Return rounded result of multiplicand*multiplier/divisor
static int32_t muldiv(int32_t, uint32_t, uint32_t) __attribute__ ((always_inline));
inline int32_t muldiv(int32_t multiplicand, uint32_t multiplier, uint32_t divisor){
    return muldivQR(multiplicand, multiplier/divisor, multiplier%divisor, divisor);
}

//Micrometer distances <==> motor step distance conversions
static int32_t um_to_steps(int32_t, uint8_t) __attribute ((always_inline));
inline int32_t um_to_steps(int32_t distance, uint8_t index){
    uint32_t steps;
    switch(index){
        case 0:
            steps = STEPS_PER_M_X;
            break;
        case 1:
            steps = STEPS_PER_M_Y;
            break;
        case 2:
            steps = STEPS_PER_M_Z;
            break;
        default: //case 3
            steps = STEPS_PER_M_E;

    }
        return muldivQR(distance, steps / 1000000UL
                steps % 1000000UL, 1000000UL);
}//end um_to_steps


//Aproximate 2d distance
uint32_t approx_distance(uint32_t dx, uint32_t dy);

//Approximate 3d distance
uint32_t approx_distance_3(uint32_t dx, uint32_t dy, uint32_t dz);

//Integer square root algo
uint16_t int_sqrt(uint32_t a);

//Crude log routin (2^ msbloc(v) >= v
const uint8_t msbloc(uint32_t v);

#endif //_DDA_MATHS
