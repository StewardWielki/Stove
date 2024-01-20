#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include "uart.h"

#define UART_TX_BUFF_SIZE   128
volatile uint8_t tailRx = 0;
volatile uint8_t headRx = 0;
uint8_t uartTxData[UART_TX_BUFF_SIZE];

#define UART_RX_BUFF_SIZE   128
volatile uint8_t tailTx = 0;
volatile uint8_t headTx = 0;
uint8_t uartRxData[UART_RX_BUFF_SIZE];

extern volatile uint8_t blowerSpeed;
extern uint8_t EEMEM eeBlowerSpeed;
extern uint8_t blowerChange;
extern uint16_t EEMEM eeMovePeriod;
extern volatile uint16_t movePeriod;

void uartInit( void )
{
    DDRD |= 0x02;
    DDRD &= ~0x1;
    UBRRH = 0;
    UBRRL = 103;//    9600
    UCSRA = 1<<U2X;
    UCSRC= 1<<URSEL | 1<<UCSZ1 | 1<<UCSZ0;
    UCSRB = 1<<TXEN | 1<<RXEN | 1<<RXCIE;
}

void uartSendByte( uint8_t data )
{
    UDR = data;
}

void uartWriteData( const char *data, uint8_t len )
{
    uint8_t tmp;

    for( uint8_t i = 0; i < len; i++ )
    {
        uartTxData[ headRx ] = data[ i ];
        UCSRB |= 1<<UDRIE;
        tmp = headRx;
        tmp++;
        tmp = tmp%UART_TX_BUFF_SIZE;
        if( tmp == tailRx )   //no space in circular buffer
        {
            uartTxData[ headRx ] = '~'; //this means overflow
            break;
        }
        else headRx = tmp;
    }
}

void SerialU( uint16_t x )
{
    char out[8];
    uint8_t i = sizeof(out);

    do
    {
        out[i-1] = x%10 + '0';
        x = x/10;
        i--;
    }while( x );
    uartWriteData( &out[i], sizeof(out)-i);
}

void SerialULn( uint16_t x )
{
    char c[] = "\n\r";

    SerialU( x );
    uartWriteData( c, sizeof(c)-1 );
}

void SerialI( int16_t x )
{
    char c = '-';
    if( x < 0 )
    {
        uartWriteData( &c, 1);
        x = -x;
    }
    SerialU((uint16_t)x);
}

void SerialILn( int16_t x )
{
    char c[] = "\n\r";

    SerialI( x );
    uartWriteData( c, sizeof(c)-1 );
}

void Serial( char *text )
{
    char * ptr = text;
    uint8_t i = 0;

    while( *ptr ) 
    {
        ptr++;
        i++;
    }
    if( i )
    {
        uartWriteData( text, i );
    }
}

void SerialLn( char *text )
{
    char c[] = "\n\r";

    Serial( text );
    uartWriteData( c, sizeof(c)-1 );
}

void SerialP( const char *text )
{
    uint8_t tmp;
    char c;

    while( (c = pgm_read_byte(text++)) )
    {
        uartTxData[ headRx ] = c;
        UCSRB |= 1<<UDRIE;
        tmp = headRx;
        tmp++;
        tmp = tmp%UART_TX_BUFF_SIZE;
        if( tmp == tailRx )   //no space in circular buffer
        {
            uartTxData[ headRx ] = '~'; //this means overflow
            break;
        }
        else headRx = tmp;
    }
}

void SerialPLn( const char *text )
{
    char c[] = "\n\r";

    SerialP( text );
    uartWriteData( c, sizeof(c)-1 );
}

ISR(USART_UDRE_vect)
{
    if( tailRx == headRx )  //no data to send
    {
        UCSRB &= ~(1<<UDRIE);
    }
    else
    {
        UDR = uartTxData[tailRx];
        tailRx++;
        tailRx = tailRx%UART_TX_BUFF_SIZE;
    }
}

// ISR(USART_RXC_vect)
// {
//     uint8_t lastData;

//     uartRxData[headRx] = UDR;

//     if(headRx) lastData = headRx - 1;
//     else lastData = UART_RX_BUFF_SIZE-1;
//     if(uartRxData[lastData] == 0x30 && uartRxData[headRx] == 0x20)
//     {
//         // //go to bootloader
//         // //WDTCR = 1<<WDE;
//         // wdt_enable(WDTO_15MS);
//         // wdt_reset();
//         // while(1);
//         DDRC |= 0x20;   //PC5
//         PORTC &= ~0x20;
//     }


//     headRx++;
//     headRx = headRx%UART_RX_BUFF_SIZE;
//     if( tailRx == headRx )
//     {
//         tailRx++;           //drop the oldest data
//         tailRx = tailRx%UART_RX_BUFF_SIZE;
//     }    
// }

ISR(USART_RXC_vect)
{
    static uint8_t lastData;
    uint8_t data;
    static uint8_t dataPtr;
    static uint8_t command;
    
    static enum
    {
        eHeader,    //0xAA
        eCommand,   //{B-blower +,-,value(0..100)}, {P-period of podajnik +,-,value}
        eData,
        eEnd,
        eLast
    } recState;

    data = UDR;

    if(lastData == 0x30 && data == 0x20)
    {
        // //go to bootloader
        // //WDTCR = 1<<WDE;
        // wdt_enable(WDTO_15MS);
        // wdt_reset();
        // while(1);
        DDRC |= 0x20;   //PC5
        PORTC &= ~0x20;
    }
    lastData = data;

    switch(recState)
    {
        default:
        case eLast:
            recState = eHeader;
        case eHeader:
            // if(data == 0xAA) recState = eCommand;
            if(data == 'S') recState = eCommand;
        break;

        case eCommand:
            if(data == 'B' || data == 'P')
            {
                command = data;
                recState = eData;
                dataPtr = 0;
            }
            else recState = eHeader;
        break;

        case eData:
            // if(data == 0x0A) recState = eEnd;
            if(data == 'E') recState = eEnd;
            else if( dataPtr > 8) recState = eHeader;
            else
            {
                uartRxData[dataPtr] = data;
                dataPtr++;
            }
        break;

        case eEnd:
            switch(command)
            {
                case 'B':
                    if(dataPtr == 2)
                    {
                        uint8_t tmp;

                        tmp = (uartRxData[0] - '0') * 10;
                        tmp += uartRxData[1] - '0';

                        //set blower value
                        blowerSpeed = tmp;
                        eeprom_write_byte( &eeBlowerSpeed, blowerSpeed);
                        blowerChange = 1;
                    }
                    else if( dataPtr == 1)
                    {
                        if(uartRxData[0] == '+')
                        {
                            blowerSpeed++;
                        }
                        else if(uartRxData[0] == '-')
                        {
                             if(blowerSpeed)blowerSpeed--;
                        }
                        else if(uartRxData[0] >= '0' && uartRxData[0] <= '9')
                        {
                             blowerSpeed = uartRxData[0] - '0';
                        }
                        eeprom_write_byte( &eeBlowerSpeed, blowerSpeed);
                        blowerChange = 1;
                    }
                    
                    
                break;

                case 'P':
                    if(dataPtr == 3)
                    {
                        uint16_t tmp;
                        
                        tmp = (uartRxData[0] - '0') * 100;
                        tmp += (uartRxData[1] - '0') * 10;
                        tmp += uartRxData[2] - '0';

                        if( tmp > MOVE_PERIOD_MAX ) tmp = MOVE_PERIOD_MAX;
                        movePeriod = tmp;
                        eeprom_write_word( &eeMovePeriod, movePeriod);
                    }
                    if(dataPtr == 2)
                    {
                        uint8_t tmp;

                        tmp = (uartRxData[0] - '0') * 10;
                        tmp += uartRxData[1] - '0';

                        //set blower value
                        movePeriod = tmp;
                        eeprom_write_word( &eeMovePeriod, movePeriod);
                    }
                    else if( dataPtr == 1)
                    {
                        if(uartRxData[0] == '+')
                        {
                            if( movePeriod < MOVE_PERIOD_MAX ) movePeriod++;
                        }
                        else if(uartRxData[0] == '-')
                        {
                             if(movePeriod)movePeriod--;
                        }
                        else if(uartRxData[0] >= '0' && uartRxData[0] <= '9')
                        {
                             movePeriod = uartRxData[0] - '0';
                        }
                        eeprom_write_word( &eeMovePeriod, movePeriod);
                    }
                    
                    
                break;
            }
            recState = eHeader;
        break;
    }
}

uint8_t uartReadData( uint8_t *data, uint8_t maxLen )
{
    uint8_t i;

    for( i=0; i<maxLen; i++)
    {
        if( headRx == tailRx ) break;
        else
        {
            data[i] = uartRxData[tailRx];
            tailRx++;
            tailRx = tailRx%UART_RX_BUFF_SIZE;
        }
    }

    return i;
}

