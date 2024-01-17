#ifndef _TIMER_H
#define _TIMER_H

#include <avr/io.h>

void timersInit( void );
uint32_t getTick( void );
void setBlowerSpeed( uint8_t duty );

#endif  //_TIMER_H
