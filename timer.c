#include "timer.h"
#include <stdlib.h>
#include <avr/interrupt.h>

volatile uint32_t tick = 0;
volatile uint16_t test;

void timer1Init( void )
{
    DDRD &= ~0x40;  //PD6 - input capture
    DDRD |= 0x40;  //PD6 - input capture
    PORTD |= 0x40;  //PD6 - pull up

    TCNT1 = 0;
    TCCR1A = 0; //NORMAL mode
    TCCR1B = 1<<ICNC1 | 1<<ICES1 | 1<<CS11 | 1<<CS10;  //Input Capture Noise Canceler, clk/64
    TIFR |= 1<<ICF1 | 1<<OCF1A | 1<<OCF1B | TOV1;
    TIMSK |= 1<<TOIE1 | 1<<OCIE1A | 1<< OCIE1B | 1<<TICIE1;

    OCR1A = 0x3FFF;
    OCR1B = 0xAFFF;
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

ISR(TIMER1_OVF_vect)
{
    test = TCNT1;
    tick += 0x200;  //top (0x10000) >> 7
}

ISR(TIMER1_COMPA_vect)
{
    
}

ISR(TIMER1_COMPB_vect)
{
    
}

ISR(TIMER1_CAPT_vect)
{
    if(TCCR1B & 1<<ICES1) TCCR1B &= ~(1<<ICES1);
    else TCCR1B |= 1<<ICES1;
    TIFR |= TICIE1; //clear flag to detect very short pulse

}
