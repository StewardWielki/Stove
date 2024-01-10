#ifndef _UART_
#define _UART_

#include <inttypes.h>

void uartInit( void );
void uartSendByte( uint8_t data );
void uartWriteData( const char *data, uint8_t len );
uint8_t uartReadData( uint8_t *data, uint8_t maxLen );
void SerialU( uint16_t x );
void SerialULn( uint16_t x );
void SerialI( int16_t x );
void SerialILn( int16_t x );
void Serial( char *text );
void SerialLn( char *text );
void SerialP( const char *text );
void SerialPLn( const char *text );

#endif  //_UART_