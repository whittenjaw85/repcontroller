#include "serial.h"

/* Serial communication is circle buffer */
/* Thanks to Teacup team... this is an attempt to figure out and apply your work to my personal electronics design. */

#include <avr/io.h>
#include <avr/interrupt.h>

//#include "memory_barrier.h"



/*Config.h and arduino not necessary... yet*/
//Must be order of 2^n
#define BUFSIZE	64

//Ascii XOFF char
#define ASCII_XOFF  19
#define ASCII_XON   17

//RxBuffer head
volatile uint8_t rxhead = 0;
//RxBuffer Tail
volatile uint8_t rxtail = 0;
//RxBuffer Variable
volatile uint8_t rxbuf[BUFSIZE];

//TxBuffer head
volatile uint8_t txhead = 0;
//TxBufferTail
volatile uint8_t txtail = 0;
//TxBuffer Variable
volatile uint8_t txbuf[BUFSIZE];

/* Unique macros for interfacing with buffers  */
//Determine if buffer is full
#define buffer_canread(buffer)	((buffer ## head - buffer ## tail ) & (BUFSIZE-1))

//Read data from buffer
#define buffer_pop(buffer, data)   do { data = buffer ## buf[buffer ## tail]; buffer ## tail = (buffer ## tail + 1) & (BUFSIZE -1); } while (0)

//Determine if the buffer can be written
#define buffer_canwrite(buffer)	((buffer ## tail - buffer ## head - 1 ) & (BUFSIZE - 1))

#define buffer_push(buffer, data) do { buffer ## buf[buffer ## head] = data; buffer ## head = (buffer ## head + 1) & (BUFSIZE -1); } while (0) 

/* Ringbuffer Discussion-------

head = written data pointer
tail = read data pointer

when the pointers are the same, the buffer is empty
when (head+1) == tail, buffer is full
available space is (tail - head) & bufsize

can write:
    (tail-head-1) & (BUFSIZE - 1)
can read:
    (head - tail) & (BUFSIZE - 1)
write to buffer:
    buf[head++] = data; head &= (BUFSIZE - 1);
read from buffer:
    data = buf[tail++]; tail &= (BUFSIZE - 1);
*/

#ifdef	XONXOFF
#define FLOWFLAG_STATE_XOFF 0
#define FLOWFLAG_SEND_XON   1
#define FLOWFLAG_SEND_XOFF  2
#define FLOWFLAG_STATE_XON  4

//Initially send an XON
volatile uint8_t flowflags = FLOWFLAG_SEND_XON;
#endif


/*ONWARD TO AMAZING FUNCTIONS   */
//Setup baud generator and interrupts, clear buffers
void serial_init(){
#if BAUD > 38401
    UCSR1A = MASK(U2X1); //asynchronous doublespeed mode
    UBRR1 = (((F_CPU / 8) / BAUD) - 0.5);
#else
    UCSR1A = 0; //set asynchronous normal mode
    UBRR! = (((F_CPU / 16) / BAUD) - 0.5); //asynchronous normal mode
#endif
    UCSR1B = MASK(RXEN1) | MASK(TXEN1); //enable rx and tx
    UCSR1C = MASK(UCSZ11) | MASK(UCSZ10); //8-bit data no parity 1-stop
    UCSR1B |= MASK(RXCIE1) | MASK(UDRIE1); //Serial Interrupts Enable
}


/*   Interrupts  */
#ifdef USART_RX_vect
ISR(USART_RX_vect)
#else
ISR(UART1_RX_vect)
#endif
{
    //Save status register
    uint8_t sreg_save = SREG;

    if (buffer_canwrite(rx)) buffer_push(rx, UDR1);
    else{
        uint8_t trash;
        //Not reading character requires discarding it
        trash = UDR1;
    }

    #ifdef XONXOFF
    if (flowflags & FLOWFLAG_STATE_XON && buffer_canwrite(rx) < 16){
    //The buffer has only 16 free characters left, so send XOFF
    // more characters may be transmitted until XOFF affects
        flowflags = FLOWFLAG_SEND_XOFF | FLOWFLAG_STATE_XON;
        //enable Tx Interrupt
        UCSR1B |= MASK(UDRIE1);
    }
    #endif

    //Restore status register
    //MEMORY BARRIER
    SREG = sreg_save;
}//end RX Interrupt ISR

//Transmit Buffer Ready Vector
#ifdef USART_UDRE_vect
ISR(USART_UDRE_vect)
#else
ISR(USART1_UDRE_vect)
#endif
{
    //Save Status
    uint8_t sreg_save = SREG;

    #ifdef XONXOFF
    if (flowflags & FLOWFLAG_SEND_XON) {
        UDR1 = ASCII_XON;
        flowflags = FLOWFLAG_STATE_XON;
    }
    else if (flowflags & FLOWFLAG_SEND_XOFF) { 
        UDR1 = ASCII_XOFF;
        flowflags = FLOWFLAG_STATE_XOFF;
    }
    else
    #endif
    if (buffer_canread(rx)) buffer_pop(tx, UDR1);
    else UCSR1B &= ~MASK(UDRIE1);

    //Memory barrier is needed sometimes: why not this one?
    //restore status register
    SREG = sreg_save;
}//End TX Interrupt Vector

//----------------------------------
// Reading Routines Below
//----------------------------------

//Determine how many chars are in the buffer
uint8_t serial_rxchars(){return buffer_canread(rx);}

//read a char
uint8_t serial_popchar()
{
    uint8_t c = 0;

    //Ensure buffer is empty, then read
    if (buffer_canread(rx))
        buffer_pop(rx, c);
#ifdef XONXOFF
    if ((flowflags & FLOWFLAG_STATE_XON) == 0 && buffer_canread(rx) < 16){
        //the buffer has (BUFSIZE - 16) free characters again, send XON
        flowflags = FLOWFLAG_SEND_XON;
        UCSR1B |= MASK(UDRIE1);
    }
#endif

    return c;
}//end readchar()

//----------------------------------
// Write routines below
//----------------------------------

//Send a char
void serial_writechar(uint8_t data)
{
    //Block interrupts if enabled
    if (SREG & MASK(SREG_I)){
        for(;buffer_canwrite(tx) == 0;) buffer_push(tx, data);
    }
    else{
        //interrupts disabled, write instead of blocking
        if (buffer_canwrite(tx))
            buffer_push(tx, data);
    }
    //Endable TX Interrupt so we can send the char
    UCSR1B |= MASK(UDRIE1);
}

//Send block of data
void serial_writeblock(void *data, int len){
    int i;
    for (i = 0; i < len ; i++) serial_writechar(((uint8_t *) data)[i]);
}//End serial_writedata

//Send string and look for null byte, no specified length
void serial_writestr(uint8_t *data){
    uint8_t i= 0, r;
    while((r = data[i++])) serial_writechar(r);
}//end serial_writestr


/*
    Write block from flash
*/
void serial_writeblock_P(PGM_P data, int len){
    int i;
    for (i = 0; i< len ; i++) serial_writechar(pgm_read_byte(&data[i]));
}//end serial_writedata_p

void serial_writestr_P(PGM_P data){
    uint8_t r, i=0;
    // Breaks when r is assigned a zero
    while ((r = pgm_read_byte(&data[i++])))	serial_writechar(r);
}//end serial_writestr_p
