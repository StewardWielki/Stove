/*
* ds18b20.c
*
* Created: 24.07.2022 16:52:41
*  Author: Clemens Hauser
*/
#include <avr/io.h>
#include "ds18b20.h"
#include <util/delay.h>
#include <avr/interrupt.h>


void bus_init()
{
	WIRE_DDR |= (1<<WIRE);						//set output
	WIRE_PORT &= ~(1<<WIRE);
	_delay_us(DELAY_RESET);
	WIRE_DDR &= ~(1<<WIRE);						//set input
	_delay_us(DELAY_RESET);
}

void bus_write_bit(char bit)
{
	WIRE_PORT &= ~(1<<WIRE);
	
	cli( );
	if(bit)
	{
		_delay_us(ONE_DUR);
		WIRE_PORT |= (1<<WIRE);
		_delay_us(ZERO_DUR);					//wait extra time to reach total timeslot duration
		
	}
	else
	{
		_delay_us(ZERO_DUR);
		WIRE_PORT |= (1<<WIRE);
	}
	sei( );
	
}

void bus_write_byte(char byte)
{
	WIRE_DDR |= (1<<WIRE);
	for(uint8_t i = 0; i<8; i++)
	{
		bus_write_bit(byte & (1<<i) );
	}
	WIRE_DDR &= ~(1<<WIRE);
}

uint8_t bus_read_bit()
{
	uint8_t result;
	cli( );
	WIRE_DDR |= (1<<WIRE);
	WIRE_PORT &= ~(1<<WIRE); //initiate timeslot
	WIRE_DDR &= ~(1<<WIRE);
	
	_delay_us(ONE_DUR);
	result = ((WIRE_PIN & (1<<WIRE))>>WIRE); 		//read bit value on bus
	sei( );
	return result;
}

uint8_t bus_read_byte()
{
	uint8_t ret=0x00;
	
	for(uint8_t i = 0; i<8; i++)
	{
		ret |= bus_read_bit() << i;				//lsb is transmitted first
		_delay_us(ZERO_DUR);
	}
	
	return ret;
}


void req_temperature()
{
	bus_init();
	bus_write_byte(ROMSKIP);
	bus_write_byte(TCONVERT);
	// bus_init();
	WIRE_PORT |= (1<<WIRE);
	WIRE_DDR |= (1<<WIRE);	//parasite power ON
}

int16_t get_temperature( void )
{
	//disable in parasite mode
	/*if(!bus_read_bit())							//check if sensor is still converting
	{
		return 0xFFFF;
	}*/

	int16_t ret=0x0000;
	//WIRE_DDR &= ~(1<<WIRE); //parasite power OFF

	bus_init();
	bus_write_byte(ROMSKIP);
	bus_write_byte(SCRATCHPADREAD);
	
	ret |= bus_read_byte();
	ret |= bus_read_byte()<<8;					//read 2nd byte
	bus_init();									//stop reading
	return ret;
}


void set_resolution(uint8_t configuration)
{	
	bus_init();
	bus_write_byte(ROMSKIP);
	bus_write_byte(SCRATCHPADWRITE);
	bus_write_byte(0x00);
	bus_write_byte(0x00);
	bus_write_byte(configuration);
	bus_init();
}

enum
{
	dsInit,
	dsSetRes,
	dsReqTemp,
	dsWaitForMeas,
	dsReadTemp,
	dsLast
} ds18b20_State;

void ds18b20_main( void )
{
	switch( ds18b20_State )
	{
		default:
		case dsInit:
		break;
	}
}
