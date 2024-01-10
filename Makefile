MCU=atmega32
F_CPU=8000000
CC=avr-gcc
OBJCOPY=avr-objcopy
CFLAGS=-std=c99 -Wall -g -O1 -mmcu=${MCU} -DF_CPU=${F_CPU} -I.
#CFLAGS=-std=c99 -Wall -g -O3 -mmcu=${MCU} -DF_CPU=${F_CPU} -I.
TARGET=main
SRCS=main.c
SRCS+=uart.c
SRCS+=lcd.c
SRCS+=timer.c
SRCS+=analog.c
SRCS+=ds18b20.c

all:
	${CC} ${CFLAGS} -o ${TARGET}.elf ${SRCS}
	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.elf ${TARGET}.hex
	$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex ${TARGET}.elf ${TARGET}.eep
	avr-size -C main.elf --mcu ${MCU}

flash:
#	avrdude -p ${MCU} -c usbasp -U flash:w:${TARGET}.hex:i -F -P usb
#	avrdude -p ${MCU} -c arduino -P COM25 -b9600 -U flash:w:${TARGET}.hex:i -F
	avrdude -p ${MCU} -c arduino -P /dev/rfcomm0 -b9600 -U flash:w:${TARGET}.hex:i -F

eeprom:
#	avrdude -p ${MCU} -c usbasp -U eeprom:w:$(TARGET).eep -F -P usb
#	avrdude -p ${MCU} -c arduino -P COM25 -b9600 -U eeprom:w:$(TARGET).eep -F
	avrdude -p ${MCU} -c arduino -P /dev/rfcomm0 -b9600 -U eeprom:w:$(TARGET).eep -F

clean:
	rm -f *.bin *.hex *.eep *.elf
