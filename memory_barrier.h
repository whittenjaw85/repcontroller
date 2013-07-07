#ifndef _MEMORY_BARRIER_H
#define _MEMORY_BARRIER_H

#include <util/atomic.h>
#include <avr/version.h>

#define MEMORY_BARRIER() __asm volatile( "" ::: "memory" )

#if __AVR_LIBC_VERSION__ < 10700UL
    #define CLI_SEI_BUG_MEMORY_BARRIER() MEMORY_BARRIER()
#else
    #define CLI_SET_BUG_MEMORY_BARRIER()
#endif

#endif //MEMORY_BARRIER_H
