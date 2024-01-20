#include "timer.h"
#include <stdlib.h>
#include <avr/interrupt.h>

volatile uint32_t tick = 0;
#define TIMERS_ACCOUNT 8
volatile uint32_t timers[ TIMERS_ACCOUNT ];
volatile uint8_t timersIndex = 0;


/*
Timers:
T0 - system tick 1 ms ( prescaler 64, top 125)
T1 - blower PWM
T2 - keyboard scan tick
*/

static void timer0Init( void )
{
    //TCCR0 = 1<<WGM01 | 1<<CS01 | 1<<CS00;  //CTC mode, prescaller = 64
    TCNT0 = 0;
    OCR0 = 125;
    TCCR0 = 1 <<WGM01 | 1<<CS01 | 1<<CS00;  //CTC mode, prescaller = 64
    TIFR |= 1<<OCF0 | 1<<TOV0;
    TIMSK |= 1<<OCIE0;
}

static void timer1Init( void )  /*10 kHz, PWM=100% == zero power, PWM=0% == max power*/
{
    TCNT1 = 0;
    TCCR1A = 1<<COM1A1 /*| 1<<COM1A0*/;
    OCR1A = 20; //2.5%
    ICR1 = 400;
    TCCR1B = 1<<WGM13 | 1<<CS10;  //Input Capture Noise Canceler, clk/1
    TIFR |= 1<<OCF1A | 1<<OCF1B | TOV1;
    TIMSK &= ~(1<<TOIE1 | 1<<OCIE1A | 1<< OCIE1B | 1<<TICIE1);
}

static void timer2Init( void )
{
    TCNT2 = 0;
    OCR2 = 78;
    TCCR2 = (1<<WGM21) | (1<<CS22) | (1<<CS21) | (1<<CS20);  //CTC mode, prescaller = 1024
    TIFR |= 1<<OCF2 | 1<<TOV2;
    TIMSK |= 1<<OCIE2;
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
    for( uint8_t i = 0; i < timersIndex; i++ )
    {
        if(timers[ i ]) timers[ i ]--;
    }
}
/*
ISR(TIMER2_OVF_vect)
{
    tick++;
}
*/
ISR(TIMER2_COMP_vect)
{
    static uint8_t cnt = 0;

    if( cnt++ > 99 )
    {
        cnt = 0;
        if( PORTD & 0x08 ) PORTD &= ~0x08;
        else PORTD |= 0x08;
    }
}

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

void setBlowerSpeed( uint8_t duty )
{
    uint16_t tmp;

    if( duty > 75 ) duty = 75;
    tmp = ICR1;
    tmp *= (uint16_t)duty;
    tmp /= 100;
    OCR1A = tmp; 
}




uint32_t* createTimer( void )
{
    uint32_t *result;

    if( timersIndex < TIMERS_ACCOUNT )
    {
        result = (uint32_t *)&timers[timersIndex];
        timersIndex++;
    }
    else result = NULL;

    return result;
}

void setTimer( uint32_t *timer, uint32_t value )
{
    *timer = value;
}

uint8_t checkIfTimerExpired( uint32_t *timer )
{
    if( *timer ) return 0;
    else return 1;
}

/*void setTimePoint( uint32_t *timePoint )
{
    *timePoint = tick;
}

uint8_t checkTimeDiff( uint32_t timePoint, uint32_t time )
{
    uint8_t result;
    uint32_t actTime;
    uint32_t tmp;

    actTime = tick;
    if(timePoint < actTime) tmp = actTime - timePoint;
    else
    {
        tmp = 0xFFFFFFFF - timePoint;
        tmp += actTime;
    }
    if( tmp  > time ) result = 1;
    else result = 0;

    return result;
}*/