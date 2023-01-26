#ifndef _TIMER_H
#define _TIMER_H

#include <avr/io.h>

void timer1Init( void );
uint32_t getTime( void );

void testISR( void );

#endif  //_TIMER_H
