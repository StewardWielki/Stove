#include "timer.h"
#include <stdlib.h>
#include <avr/interrupt.h>

volatile uint32_t tick = 0;

/*
Timers:
T0 - system tick 1 ms ( prescaler 64, top 125)
T1 - blower PWM
T2 - keyboard scan tick
*/

static void timer0Init( void )
{
    TCCR0 = 1<<WGM01 | 1<<CS01 | 1<<CS00;  //CTC mode, prescaller = 64
    TCNT0 = 0;
    OCR0 = 125;
    TCCR0 = 1 <<WGM01 | 1<<CS01 | 1<<CS00;  //CTC mode, prescaller = 64
    TIFR |= 1<<OCF0 | 1<<TOV0;
    TIMSK |= 1<<OCIE0;
}

static void timer1Init( void )
{
/*   TCNT1 = 0;
    TCCR1A = 0; //NORMAL mode
    TCCR1B = 1<<ICNC1 | 1<<ICES1 | 1<<CS11 | 1<<CS10;  //Input Capture Noise Canceler, clk/64
    TIFR |= 1<<ICF1 | 1<<OCF1A | 1<<OCF1B | TOV1;
    TIMSK |= 1<<TOIE1 | 1<<OCIE1A | 1<< OCIE1B | 1<<TICIE1;

    OCR1A = 0x3FFF;
    OCR1B = 0xAFFF;*/
}

static void timer2Init( void )
{
/*    TCNT2 = 0;
    OCR2 = 100;
    TCCR2 = (1<<WGM21) | (1<<CS22);  //CTC mode, prescaller = 64
    // TCCR2 = 0x08;
    // TCCR2 = 1<<CS22;  //CTC mode, prescaller = 64
    TIFR |= 1<<OCF2 | 1<<TOV2;
    TIMSK |= 1<<OCIE2;
    */
}

void timersInit( void )
{
    timer0Init( );
    timer1Init( );   
    timer2Init( );
}


ISR(TIMER0_COMP_vect)
{
    tick++;
}
/*
ISR(TIMER2_OVF_vect)
{
    tick++;
}

ISR(TIMER2_COMP_vect)
{
    tick++;
}*/

uint32_t getTick( void )
{
    return tick;
}

/*ISR(TIMER1_CAPT_vect)       //max 800 uP cycles
{
    
}

ISR(TIMER1_OVF_vect)
{
    test = TCNT1;
    tick += 0x200;  //top (0x10000) >> 7
}

ISR(TIMER1_COMPA_vect)
{
    //enable gate current
}

ISR(TIMER1_COMPB_vect)
{
    //disable gate current
}
*/