#ifndef _DDA_QUEUE_H
#define _DDA_QUEUE_H

#include "dda.h"
#include "timer.h"

#define HEATER_WAIT_TIMEROUT 1000 MS

//Ringbuffer for current and pending moves
extern uint8_t mb_head;
extern uint8_t mb_tail;
extern DDA movebuffer[MOVEBUFFER_SIZE];

//Methods
uint8_t queue_full(void);
uint8_t queue_empty(void);

//take a step
void queue_step(void);

//Add target to queue, NULL means to wait for target temp to the queue
void enqueue(TARGET *t);
void enqueue_home(TARGET *t, uint8_t endstop_check, uint8_t endstop_stop_cond);

//called from step timer when current move is complete
void next-Move(void) __attribute__ ((hot));

//print queue status
void print_queue(void);

//flush
void queue_flush(void);

//wait for queue to empty
void queue_wait(void);

#endif //DDA_QUEUE_H
