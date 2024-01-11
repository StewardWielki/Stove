#include "timer.h"
#include <stdlib.h>
#include <avr/interrupt.h>

volatile uint32_t tick = 0;
volatile uint16_t test;

typedef enum
{
    FALLING = 0,
    RASING = 1
}Edge_t;

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

volatile uint16_t risingEdge[4];
volatile uint16_t fallingEdge[4];
volatile uint8_t edgeItem = 0;
volatile uint8_t edgeItemLast = 0x03;
volatile uint8_t edgeItemPrevious = 0x02;
volatile uint8_t edgeCnt;
volatile uint16_t period = 0;
volatile uint16_t deadTime = 0;
volatile uint16_t startAngle = 0;

void calcPeriod( int8_t edge )
{
    uint16_t tmp;

    if( edge )
    {
        if( risingEdge[edgeItemLast] >= risingEdge[edgeItemPrevious] ) tmp = risingEdge[edgeItemLast] - risingEdge[edgeItemPrevious];
        else tmp = 0xFFFF - risingEdge[edgeItemPrevious] + 1 + risingEdge[edgeItemLast];
    }
    else
    {
        if( fallingEdge[edgeItemLast] >= fallingEdge[edgeItemPrevious] ) tmp = fallingEdge[edgeItemLast] - fallingEdge[edgeItemPrevious];
        else tmp = 0xFFFF - fallingEdge[edgeItemPrevious] + 1 + fallingEdge[edgeItemLast];
    }
    if( period == 0 ) period = tmp;
    else
    {
        if( tmp > period ) period += (tmp - period) >> 3;
        else  period -= (period - tmp) >> 3;
    }
}

void calcDeadTime( Edge_t edge )
{
    uint16_t tmp, halfPeriod;

    if( edge == FALLING )
    {
        if( fallingEdge[edgeItemLast] >= fallingEdge[edgeItemPrevious] ) halfPeriod = fallingEdge[edgeItemLast] - fallingEdge[edgeItemPrevious];
        else halfPeriod = 0xFFFF - fallingEdge[edgeItemPrevious] + 1 + fallingEdge[edgeItemLast];
        halfPeriod >>= 1;  //divide by 2 - half of period
        if( fallingEdge[edgeItemLast] >= risingEdge[edgeItemLast] ) tmp = fallingEdge[edgeItemLast] - risingEdge[edgeItemLast];
        else tmp = 0xFFFF - risingEdge[edgeItemLast] + 1 + fallingEdge[edgeItemLast];
        if( halfPeriod > tmp )  //it must be !
        {
            tmp = ( halfPeriod - tmp ) >> 1;
        }
        else
        {
            //TODO
            //tmp = default deadtime;
            //increase error counter
        }
        if( deadTime == 0 ) deadTime = tmp;
        else
        {
            if( tmp > deadTime ) deadTime += (tmp - deadTime) >> 3;
            else deadTime -= (deadTime - tmp) >> 3;
        } 
    }
}

uint16_t calcZeroPoint( Edge_t edge )
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

#define TRIAC_GATE_ON_TIME (480/8)    //480 us / (64/8)
#define TURN_OFF_MARGIN (480/8)
ISR(TIMER1_CAPT_vect)       //max 800 uP cycles
{
    Edge_t edge;
    uint16_t capture = ICR1;
    uint16_t zeroPoint;

    edge = (TCCR1B & 1<<ICES1) ? RASING : FALLING;
    if(edge == RASING) TCCR1B &= ~(1<<ICES1);
    else TCCR1B |= 1<<ICES1;
    TIFR |= TICIE1; //clear flag to detect very short pulse

    if(edge == RASING) risingEdge[edgeItem] = capture;
    else fallingEdge[edgeItem] = capture;
    edgeItemPrevious = edgeItemLast;
    edgeItemLast = edgeItem;
    edgeItem = (edgeItem+1) & 0x03;   //pointer to 4 elements
    calcPeriod( edge );
    calcDeadTime( edge );
    zeroPoint = calcZeroPoint( edge );
    OCR1A = zeroPoint + startAngle;    //triac turn on, startAngle is power
    //OCR1B = zeroPoint + (period>>1 - TURN_OFF_MARGIN);    //triac turn off
    OCR1B = OCR1A + TRIAC_GATE_ON_TIME;    //triac turn off
    //TODO check if period and dead time have a proper value, in specific range !
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
