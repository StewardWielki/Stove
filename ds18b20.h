/*
* ds18b20.h
*
* Created: 24.07.2022 16:53:12
*  Author: Clemens Hauser
*/


#ifndef DS18B20_H_
#define DS18B20_H_

//#define F_CPU 8000000				//MODIFY TO YOUR CLOCK! IMPORTANT!

#define WIRE		PA5				//Pin One-Wire is connected to
#define WIRE_DDR	DDRA
#define WIRE_PORT	PORTA
#define WIRE_PIN	PINA

//ROM commands

#define ROMSEARCH	0xF0
#define ROMREAD		0x33
#define ROMMATCH	0x55
#define ROMSKIP		0xCC
#define ALARMSEARCH 0xEC

//Function commands

#define TCONVERT		0x44
#define SCRATCHPADWRITE 0x4E
#define SCRATCHPADREAD	0xBE
#define SCRATCHPADCOPY	0x48
#define RECALLE			0xB8
#define READPOWER		0xB4

//TIMINGS in us

#define DELAY_RESET			500
#define TIMESLOT_DUR		70
#define ONE_DUR				10
#define ZERO_DUR			70

// //Correction factor fixed point arithmetic
// #define CORRECTION12		6		//for 12-Bit mode, exact value:		6.25
// #define CORRECTION11		12		//for 11-Bit mode, exact value:		12.5
// #define CORRECTION10		25		//for 10-Bit mode, exact value:		25
// #define CORRECTION9			50		//for 9-Bit mode, exact value:		50

// //Correction shift due to fixed point arithmetic
// #define SHIFT12				0		
// #define SHIFT11				1		
// #define SHIFT10				2		
// #define SHIFT9				3

//Configuration Bytes
#define CONFIG12	0x7F				//12 Byte-Mode
#define CONFIG11	0x5F				//11 Byte-Mode
#define CONFIG10	0x3F				//10 Byte-Mode
#define CONFIG9		0x1F				//9 Byte-Mode


void bus_init();

void bus_write_bit(char bit);
void bus_write_byte(char byte);

uint8_t bus_read_bit();
uint8_t bus_read_byte();

void set_resolution(uint8_t configuration);						// set configuration bytes for 12 bit, 11 bit...

void req_temperature();											//start sensor temperature conversion
int16_t get_temperature( void );		//returns converted temperature

#endif /* DS18B20_H_ */