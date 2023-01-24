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
    //enable gate current
}

ISR(TIMER1_COMPB_vect)
{
    //disable gate current
}


volatile uint16_t risingEdge[4];
volatile uint16_t fallingEdge[4];
volatile uint8_t edgeItem = 0;
volatile uint8_t edgeCnt;
volatile uint16_t period = 0;
volatile uint16_t deadTime = 0;
volatile uint16_t startAngle = 0;

void calcPeriod( int8_t edge )
{
    uint16_t tmp;
    uint8_t i;

    i = edgeItem - 1;
    i &= 0x03;
    if( edge )
    {
        if( risingEdge[i] > risingEdge[edgeItem] ) tmp = risingEdge[i] - risingEdge[edgeItem];
        else tmp = risingEdge[edgeItem] - risingEdge[i];
    }
    else
    {
        if( fallingEdge[i] > fallingEdge[edgeItem] ) tmp = fallingEdge[i] - fallingEdge[edgeItem];
        else tmp = fallingEdge[edgeItem] - fallingEdge[i];
    }
    if( period == 0 ) period = tmp;
    else
    {
        if( tmp > period ) period += (tmp - period) >> 3;
        else  period -= (period - tmp) >> 3;
    }
}

void filterDeadTime( int16_t newDeadTime )
{
    if( deadTime == 0 ) deadTime = newDeadTime;
    else
    {
        if( newDeadTime > deadTime ) deadTime += (newDeadTime - deadTime) >> 3;
        else deadTime -= (deadTime - newDeadTime) >> 3;
    }
}

uint16_t calcZeroPoint( uint8_t edge )
{
    uint8_t i;
    uint8_t index;
    uint8_t prevIndex;
    uint8_t overflow = 0;
    uint32_t sum;
    volatile uint16_t * capData;

    if( edge ) capData = risingEdge;
    else capData = fallingEdge;
    sum = period;
    sum *= 6;
    index = edgeItem;  //edgeItem shows place where will be write new capture value - it is also the oldest captured data - circular buffer
    sum += capData[index];    //first part of average
    for( i=0; i<3; i++ )        //tree parts of average
    {
        prevIndex = index;
        index = (index+1) & 0x03;
        if( !overflow && capData[ index ] < capData[ prevIndex ] ) overflow = 1; //it means that overflow happen
        if( overflow ) sum += 0x10000;
        sum += capData[index];
    }
    sum >>= 2;  //divide by 4
    if( edge ) sum -= deadTime; //sub dead time for rising edge
    else sum += deadTime;   //add dead time for falling edge
    return (uint16_t)sum;
}

ISR(TIMER1_CAPT_vect)
{
    uint8_t edge;
    uint16_t capture = ICR1;
    uint16_t compare;

    edge = (TCCR1B & 1<<ICES1) ? 1 : 0;
    if(edge) TCCR1B &= ~(1<<ICES1);
    else TCCR1B |= 1<<ICES1;
    TIFR |= TICIE1; //clear flag to detect very short pulse

    if(edge) risingEdge[edgeItem] = capture;
    else fallingEdge[edgeItem] = capture;
    edgeItem = (edgeItem+1) & 0x03;   //pointer to 4 elements
    calcPeriod( edge );
    compare = calcZeroPoint( edge );
    compare += startAngle;
    if( edge ) OCR1A = compare;
    else OCR1B = compare;
    // remember to calculate dead time, check if period and dead time have a proper value
}
