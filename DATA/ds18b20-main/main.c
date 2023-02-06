/*
* ds18b20.c
*
* Created: 24.07.2022 16:50:12
* Author : Clem
*/

#include <avr/io.h>
#include "lcd-routines.h"
#include "ds18b20.h"
#include <util/delay.h>
#include <stdlib.h>

int main(void)
{

	
	char buffer[16];
	lcd_init();
	lcd_string("init");
	_delay_ms(2000);
	lcd_clear();
	lcd_home();
	
	set_resolution(CONFIG12);
	
	int32_t temp;
	
	while (1)
	{
		req_temperature();
		_delay_ms(500);							//wait conversion time
		temp = get_temperature(CORRECTION12, SHIFT12);
		
		if(temp == 0xFFFF)
		{
			lcd_string("Conv in progress");
		}
		else
		{
			lcd_setcursor(0,2);
			lcd_string(itoa(temp,buffer,10));
			lcd_string(" G  ");
			lcd_home();
			lcd_string(itoa(temp,buffer,2));
		}
	}
}

