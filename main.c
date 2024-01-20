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
uint8_t EEMEM eeBlowerSpeed = 5;
uint16_t EEMEM eeMovePeriod = 60;

volatile uint8_t blowerChange = 1;
volatile uint8_t blowerSpeed = 0;

volatile uint8_t movePeriodChange = 1;
volatile uint16_t movePeriod = 59;

/* User defined characters */
const uint8_t PROGMEM thermometer[] = {4,10,10,10,17,31,31,14};


uint32_t *timerLed;
uint32_t *timerDebug;

uint32_t *timerMovePeriod;
#define MOVE_TIME   1000
uint32_t *timerMoveTime;


/*
PA03 - wipers motor - posuw


39  PA1 pompa C.O.
38  PA2 zasilacz
37  PA3 podajnik
36  PA4 triak
35  PA5 krańcówka
34  PA6 krańcówka
33  PA7 przepływomierz
28  PC6 impulsator
19  PD5 dmuchawa

*/


int main (void)
{
    //int a = -10;
    //uint8_t a2;
    
    
    //PD5
    DDRD |= 0x38;
    PORTD |= 0x38;

    DDRA |= 1<<PA3 | 1<<PA2 | 1<<PA1;
    PORTA = 1<<PA3 | 1<<PA2 | 1<<PA1;

    timersInit( );
    uartInit();
    lcd_init();
    analogInit();
    // sei( );
    lcd_backlight( 1 );
    lcd_createChar_P(0, thermometer);

    lcd_home();

    lcd_str_P(PSTR("Sterownik pieca"));
    //lcd_uint16(a2);

    blowerSpeed = eeprom_read_byte(&eeBlowerSpeed);
    movePeriod = eeprom_read_word(&eeMovePeriod);
    if( movePeriod > MOVE_PERIOD_MAX ) movePeriod = MOVE_PERIOD_MAX;
    

sei( );

    // setBlowerSpeed(10);
    //setTimePoint( &timerLed );
    //setTimePoint( &timerDebug );
    timerLed = createTimer( );
    timerDebug = createTimer( );
    timerMovePeriod = createTimer( );
    timerMoveTime = createTimer( );
    setTimer( timerLed, 500);
    setTimer( timerDebug, 1000);
    setTimer( timerMovePeriod, ((uint32_t)movePeriod)*1000 );

    while( 1 )
    {
        if( blowerChange )
        {
            blowerChange = 0;
            setBlowerSpeed( blowerSpeed );
            lcd_setCursor(0,1);
            lcd_str_P(PSTR("Blow      "));
            lcd_setCursor(5,1);
            lcd_uint16(blowerSpeed);
            lcd_str_P(PSTR(" %"));
        }
        if( movePeriodChange )
        {
            movePeriodChange = 0;
          
        }
        if( checkIfTimerExpired(timerMovePeriod) )
        {
            uint32_t tmp;

            tmp = movePeriod;
            tmp *= 1000;
            setTimer( timerMovePeriod, tmp );
            PORTA &= ~(1<<3);
            setTimer( timerMoveTime, MOVE_TIME );
        }
        if( checkIfTimerExpired(timerMoveTime) )
        {
            PORTA |= 1<<3;
        }
        if( checkIfTimerExpired(timerLed) )
        {
            PORTD ^= 0x10;
            setTimer( timerLed, 500);
        }
        if( checkIfTimerExpired(timerDebug) )
        {
            uint16_t tmp;

            SerialP(PSTR("Blower: ")); SerialU(blowerSpeed);
            SerialP(PSTR(" Period: ")); SerialILn(movePeriod);

            tmp = (uint16_t)(*timerMovePeriod / 1000);
            //tmp /= 1000;
            lcd_setCursor(0,0);
            lcd_str_P(PSTR("T               "));
            lcd_setCursor(2,0);
            lcd_uint16(tmp);
            lcd_str_P(PSTR(" s P "));
            lcd_uint16(movePeriod);
            lcd_str_P(PSTR(" s"));

            setTimer( timerDebug, 1000);
        }
        /*if( checkTimeDiff(timerLed, 500) )
        {
            //PORTD ^= 0x10;
            if( PORTD & 0x10 ) PORTD &= ~0x10;
            else PORTD |= 0x10;
            setTimePoint( &timerLed );
        }
        if( checkTimeDiff( timerDebug, 1000 ) )
        {
            SerialP(PSTR("Blower: ")); SerialILn(blowerSpeed);
            setTimePoint( &timerDebug );
        }*/
    }
    

    // while(1)
    // {
    //     // setBlowerSpeed(3);
    //     // PORTA &= ~(1<<3);
    //     PORTD &= ~0x10;
    //     _delay_ms (500);
        
    //     // setBlowerSpeed(30);
    //     PORTD |= 0x10;
    //     // PORTA |= 1<<3;
    //     _delay_ms (500);


    //     //blowerSpeed = getTick();
    //     lcd_setCursor(0,1);
    //     lcd_str_P(PSTR("Blow "));
    //     /*lcd_int16( blowerSpeed/10 );
    //     lcd_str_P(PSTR("."));
    //     lcd_int16( blowerSpeed%10 );
    //     lcd_str_P(PSTR(" C"));*/
    //     lcd_uint16(blowerSpeed);
    //     setBlowerSpeed( blowerSpeed );
        
    //     SerialP(PSTR("Pamiec flash "));  SerialILn(5);
    //     // lcd_setCursor(0,1);
    //     // lcd_str("    ");
    //     // lcd_setCursor(0,1);
    //     // //lcd_int16(a);
    //     // lcd_uint32( getTime( ) );
    //     // lcd_char(0);
    //     // SerialP(PSTR("Tick ")); SerialULn( getTime( ) );
    //     // a++;
    //     // {
    //     //     set_resolution(CONFIG12);
    //     //     req_temperature();
    //     //     _delay_ms(1000);							//wait conversion time
    //     //     blowerSpeed = get_temperature( );
            
    //     //     /*if(blowerSpeed == 0xFFFF)
    //     //     {
    //     //         SerialP(PSTR("DS18B20: not ready"));
    //     //     }
    //     //     else
    //     //     {*/
    //     //     blowerSpeed *= 10;
    //     //     blowerSpeed >>= 4;
    //     //         SerialP(PSTR("DS18B20: ")); SerialILn(blowerSpeed);
    //     //     /*}*/
    //     // }
    // }
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
sudo service bluetooth restart
avrdude -p m32 -c arduino -P /dev/rfcomm0 -b 9600 -U flash:w:main.hex
*/
