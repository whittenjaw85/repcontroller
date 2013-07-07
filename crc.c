#include "crc.h"
#include <util/crc16.h>

//block and once crc calculator, uses avr-lib crc16 optimized routine
uint16_t crc_block(void *data, uint16_t len){
    uint16_t crc = 0;
    for(; len; data++, len--)
    {
        crc = _crc16_update(crc, *((uint8_t *) data));
    }
    return crc;
}
