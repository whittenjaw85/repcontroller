#include "gcode.h"

#include "serial.h"

void gcode_init(){
    next_target.seen_G = 1;
    next_target.G = 1;
}//end gcode init

// State variables
enum{
    MOTION = 0,
    CONTROL,
    UNKNOWN
};

enum{
    GFLAG = 0,
    TFLAG,
    MFLAG,
    XFLAG,
    YFLAG,
    ZFLAG,
    SYMBOLFLAG
};

uint8_t state = UNKNOWN;

void gcode_parsechar(uint8_t c){
    // Uppercase
    if(c >= 'a' && c <= 'z')
        c &= ~32; 

    if(c=='\n')
        gcode_processbuffer();
    else{
        switch(state){
            case MOTION:
                switch(c){
                    case 'G':
                    case 'M':
                    //Linear Motion 
                    state = MOTION;
                    break;
                }//switch case c MOTION
            break;
            case SETTINGS:
        }//end switch state 
    }//end else '\n'

        case 'X':

        break;
        case 'Y':

        break;
        case 'Z':

        break;
        case 'E':

        break;
        case 'F':

        break;
        case 'S':

        break;
        case 'P':

        break;
        case 'T':

        break;
        case 'N':

        break;
        default://numeric processing
        //Ensure that the input character is numeric
        if(c >= '0' && c <= '9')
        {

        }

    }//switch

}//end gcode parsechar

