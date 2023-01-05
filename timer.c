#include "timer.h"
#include <stdlib.h>
#include <avr/interrupt.h>

volatile uint32_t tick;

void timer1Init( void )
{
    TCCR1A = 0; //NORMAL mode
    TCCR1B = 1<<ICNC1 | 1<<CS11 | 1<<CS10;  //Input Capture Noise Canceler, clk/64
    TIFR |= 1<<ICF1 | 1<<OCF1A | 1<<OCF1B | TOV1;
    TIMSK |= TOIE1;
}

uint32_t getTick( void )
{
    return tick;
}


ISR(TIMER1_OVF_vect)
{
    tick++;
}

