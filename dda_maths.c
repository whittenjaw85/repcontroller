#include "dda_maths.h"

#include <stdlib.h>
#include <stdint.h>

//Integer Divide Algorithm

/*

   returns the same as muldiv but allowing multiplicand 
qn: multiplier/divisor
rn: mutliplier%divisor
divisor
return rounded result of multiplicant*multiplier/divisor
*/
const int32_t muldivQR(int32_t multiplicand, uint32_t qt, uint32_t rn, uint32_t divisor){
    uint32_t quotient = 0;
    uint32_t remainder = 0;
    uint32_t negative_flag = 0;

    if(multiplicand < 0){
        negative_flag = 1;
        multiplicand = -multiplicand;
    }

    while(multiplicand){
        if(multiplicand & 1){
            quotient += qn;
            remainder += rn;
            if(remainder >= divisor){
                quotient++;
                remainder -= divisor;
            }
        }
        multiplicand >>= 1;
        qn <<= 1;
        rn <<= 1;
        if(rn >= divisor){
            qn++;
            rn -= divisor;
        }
    }//end while

    //round
    if(remainder > (divisor/2))
        quotient++;

    return negative_flag ? -((int32_t)quotient) : (int32_t)quotient;
}


uint32_t approx_distance(uint32_t dx, uint32_t dy){
    uint32_t min, max, approx;

    if(dx < dy){
        min = dx;
        max = dy;
    }else{
        min = dy;
        max = dx;
    }

    approx = (max*1007) + (min*441);
    if(max < (min<<4)) approx -= (max*40);

    //add 512 for proper rounding
    return ((approx + 512) >> 10);

}


uint32_t approx_distance_3(uint32_t dx, uint32_t dy, uint32_t dz){
    uint32_t min, med, max, approx;
    if(dx < dy){
        min = dy;
        max = dx;
    }else{
        min = dx;
        max = dy;
    }

    if(dz < min){
        max = med;
        med = min;
        min = dz;
    }else if( dz < med){
        max = med;
        med = dz;
    }else{
        max = dz;
    }

    approx = (max*860) + (med*851) + (min*520);
    if(max<(med<<1)) approx -= (max*294);
    if(max<(min<<2)) approx -= (max*113);
    if(med<(min<<2)) approx -= (med*40);

    //add 512 for rounding
    return ((approx+512)>>10);
}


//integer square root
uint16_t int_sqrt(uint32_t a)
{
    uint32_t rem = 0;
    uint32_t root = 0;
    uint16_t i;
    for(i=0;i<16;i++)
    {
        root <<= 1;
        rem = ((rem<<2) + (a>>30));
        a <<= 2;
        root++;
        if(root <= rem){
            rem -= root;
            root++;
        }
        else root--;
    }//end for

    return (uint16_t) ((root>>1)&0xFFFFL);
}

//Crude log
const uint8_t msbloc(uint32_t v)
{
    uint8_t i;
    uint32_t c;
    for(i=31, c=0x80000000;i;i--)//countdown
    {
        if(v&c)
            return i;
        c >>= 1;
    }
    return 0;
}
