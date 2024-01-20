#ifndef _TIMER_H
#define _TIMER_H

#include <avr/io.h>

void timersInit( void );
uint32_t getTick( void );
void setBlowerSpeed( uint8_t duty );
//void setTimePoint( uint32_t *timePoint );
//uint8_t checkTimeDiff( uint32_t timePoint, uint32_t time );
uint32_t* createTimer( void );
void setTimer( uint32_t *timer, uint32_t value );
uint8_t checkIfTimerExpired( uint32_t *timer );

#endif  //_TIMER_H
