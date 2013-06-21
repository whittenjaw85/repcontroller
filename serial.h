#ifndef _SERIAL_H
#define _SERIAL_H

#include "config.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>


// Initialize serial port communication
void serial_init(void);

//Return num of characters in the Rx Buffer and 'spaces' in the Tx Buffer
uint8_t serial_rxchars(void);
//uint8_t serial_txchars(void);

//Read char
uint8_t serial_popchar(void);

//Write char
void serial_writechar(uint8_t chr);

//Write data
void serial_writeblock(void *data, int len);
void serial_writestr(uint8_t *str);

//Write stored data
void serial_writeblock_P(PGM_P data, int len); //P is for the buffer (pointer)
void serial_writestr_P(PGM_P data);

#endif /* _SERIAL_H */
