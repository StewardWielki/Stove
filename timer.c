#include "timer.h"
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

volatile uint32_t tick = 0;
volatile uint16_t test;

void timer1Init( void )
{
    TCNT1 = 0;
    TCCR1A = 0; //NORMAL mode
    TCCR1B = 1<<ICNC1 | 1<<CS11 | 1<<CS10;  //Input Capture Noise Canceler, clk/64
    TIFR |= 1<<ICF1 | 1<<OCF1A | 1<<OCF1B | TOV1;
    TIMSK |= 1<<TOIE1;
}

uint32_t getTime( void )
{
    uint32_t result;
    uint32_t tmp;

    cli();
    tmp = TCNT1;
    result = tick;
    if( TIFR & 1<<TOV1)
    {
        tmp = TCNT1;    //read again to bee sure that this is actual value
        result += 0x200;
    }
    sei();
    result += tmp >> 7;

    return result;
}

uint16_t getTest( void )
{
    return test;
}

ISR(TIMER1_OVF_vect)
{
    test = TCNT1;
    tick += 0x200;  //top (0x10000) >> 7
}

