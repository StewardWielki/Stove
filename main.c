#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include "uart.h"
#include "lcd.h"
#include "timer.h"
#include "analog.h"

uint8_t EEMEM testE = 23;
/* User defined characters */
const uint8_t PROGMEM thermometer[] = {4,10,10,10,17,31,31,14};

int main (void)
{
    //int a = -10;
    uint8_t a2;
    int32_t temp=0;
    
    //PD5
    DDRD |= 0x38;
    PORTD |= 0x38;
    timersInit( );
    uartInit();
    lcd_init();
    analogInit();
    // sei( );
    lcd_backlight( 1 );
    lcd_createChar_P(0, thermometer);

    lcd_home();

    lcd_str_P(PSTR("Sterownik "));
    a2=eeprom_read_byte (&testE);
    lcd_uint16(a2);



    

sei( );

    

    while(1)
    {
        setBlowerSpeed(0);
        PORTD &= ~0x10;
        _delay_ms (500);
        
        setBlowerSpeed(100);
        PORTD |= 0x10;
        _delay_ms (500);


        temp = getTick();
        lcd_setCursor(0,1);
        lcd_str_P(PSTR("Temp "));
        lcd_int16( temp/10 );
        lcd_str_P(PSTR("."));
        lcd_int16( temp%10 );
        lcd_str_P(PSTR(" C"));

        SerialP(PSTR("Pamiec flash "));  SerialILn(5);
        // lcd_setCursor(0,1);
        // lcd_str("    ");
        // lcd_setCursor(0,1);
        // //lcd_int16(a);
        // lcd_uint32( getTime( ) );
        // lcd_char(0);
        // SerialP(PSTR("Tick ")); SerialULn( getTime( ) );
        // a++;
        // {
        //     set_resolution(CONFIG12);
        //     req_temperature();
        //     _delay_ms(1000);							//wait conversion time
        //     temp = get_temperature( );
            
        //     /*if(temp == 0xFFFF)
        //     {
        //         SerialP(PSTR("DS18B20: not ready"));
        //     }
        //     else
        //     {*/
        //     temp *= 10;
        //     temp >>= 4;
        //         SerialP(PSTR("DS18B20: ")); SerialILn(temp);
        //     /*}*/
        // }
    }
}

/*avrdude: safemode: Fuses OK (E:FF, H:D6, L:A4)
avrdude: safemode: lfuse changed! Was 0, and is now a4
Would you like this fuse to be changed back? [y/n] y
avrdude: safemode: and is now rescued
avrdude: safemode: hfuse changed! Was 0, and is now d6
Would you like this fuse to be changed back? [y/n] y
avrdude: safemode: and is now rescued
avrdude: safemode: Fuses OK (E:FF, H:00, L:00)*/


/*
sudo bluetoothctl
#power on
#agent on
#scan on
#scan of
sudo killall rfcomm
sudo rfcomm connect /dev/rfcomm0 20:15:10:12:01:58 1 &
avrdude -p m32 -c arduino -P /dev/rfcomm0 -b 9600 -U flash:w:main.hex
*/
