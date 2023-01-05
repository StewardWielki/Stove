#include "timer.h"
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

volatile uint32_t tick = 10;

void timer1Init( void )
{
    TCNT1 = 0;
    TCCR1A = 0; //NORMAL mode
    TCCR1B = 1<<ICNC1 | 1<<CS11 | 1<<CS10;  //Input Capture Noise Canceler, clk/64
    TIFR |= 1<<ICF1 | 1<<OCF1A | 1<<OCF1B | TOV1;
    TIMSK |= 1<<TOIE1;
}

uint32_t getTick( void )
{
    return tick;
}

uint32_t getTime( void )
{
    uint32_t result;
    uint64_t tmp;

    tmp = tick;
    tmp = (tmp * 524288)/1000;
    result = (uint32_t)tmp;
    result += TCNT1 / 125;

    return result;
}

ISR(TIMER1_OVF_vect)
{
    tick++;
}

