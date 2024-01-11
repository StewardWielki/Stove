#ifndef _TIMER_H
#define _TIMER_H

#include <avr/io.h>

void timersInit( void );
uint32_t getTick( void );

#endif  //_TIMER_H
