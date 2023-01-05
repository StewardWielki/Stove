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

uint8_t EEMEM testE = 25;
/* User defined characters */
const uint8_t PROGMEM thermometer[] = {4,10,10,10,17,31,31,14};

int main (void)
{
    int a = -10;
    uint8_t a2;
    //PD5
    DDRD |= 0x38;
    PORTD = 0x38;
    timer1Init( );
    uartInit();
    lcd_init();
    sei( );
    lcd_backlight( 1 );
    lcd_createChar_P(0, thermometer);

    lcd_home();

    lcd_str_P(PSTR("Sterownik "));
    a2=eeprom_read_byte (&testE);
    lcd_uint16(a2);

    while(1)
    {
        _delay_ms (500);
        PORTD &= ~0x10;
        _delay_ms (500);
        PORTD |= 0x10;

        SerialP(PSTR("Pamiec flash "));  SerialILn(a);
        lcd_setCursor(0,1);
        lcd_str("    ");
        lcd_setCursor(0,1);
        //lcd_int16(a);
        lcd_uint32( getTick( ) );
        lcd_char(0);
        SerialP(PSTR("Tick ")); SerialULn( getTick( ) );
        a++;
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