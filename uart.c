#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define UART_TX_BUFF_SIZE   128
volatile uint8_t tail = 0;
volatile uint8_t head = 0;
uint8_t uartData[UART_TX_BUFF_SIZE];

void uartInit( void )
{
    DDRD |= 0x02;
    DDRD &= ~0x1;
    UBRRH = 0;
    UBRRL = 8;
    UCSRA = 1<<U2X;
    UCSRC= 1<<URSEL | 1<<UCSZ1 | 1<<UCSZ0;
    UCSRB = 1<<TXEN | 1<<RXEN;
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
        uartData[ head ] = data[ i ];
        UCSRB |= 1<<UDRIE;
        tmp = head;
        tmp++;
        tmp = tmp%UART_TX_BUFF_SIZE;
        if( tmp == tail )   //no space in circular buffer
        {
            uartData[ head ] = '~'; //this means overflow
            break;
        }
        else head = tmp;
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
        uartData[ head ] = c;
        UCSRB |= 1<<UDRIE;
        tmp = head;
        tmp++;
        tmp = tmp%UART_TX_BUFF_SIZE;
        if( tmp == tail )   //no space in circular buffer
        {
            uartData[ head ] = '~'; //this means overflow
            break;
        }
        else head = tmp;
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
    if( tail == head )  //no data to send
    {
        UCSRB &= ~(1<<UDRIE);
    }
    else
    {
        UDR = uartData[tail];
        tail++;
        tail = tail%UART_TX_BUFF_SIZE;
    }
}
