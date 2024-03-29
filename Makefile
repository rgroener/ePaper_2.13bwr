# Name: Makefile
# Project: testing buttons Sequenzystem
# Author: Christian Starkjohann
# Creation Date: 2004-12-29
# Tabsize: 4
# Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt) or proprietary (CommercialLicense.txt)
# This Revision: $Id: Makefile 277 2007-03-20 10:53:33Z cs $
#atmega644 fuer Serienbord

DEVICE     = atmega328p
CLOCK      = 8000000
PROGRAMMER = -c usbasp -P usb
AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE) -v

# avrdude -c usbasp -P usb -p atmega644p -U lfuse:w0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m

# Choose your favorite programmer and interface above.

COMPILE = avr-gcc -Wall -Os -Iusbdrv -I. -mmcu=$(DEVICE) #-DDEBUG_LEVEL=2
# NEVER compile the final product with debugging! Any debug output will
# distort timing so that the specs can't be met.

OBJECTS =  main.o 


# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

program:	all
	$(AVRDUDE) -U flash:w:main.hex:i

clean:
	rm -f main.hex main.lst main.obj main.cof main.list main.map main.eep.hex main.bin *.o main.s

# file targets:
main.bin:	$(OBJECTS)
	$(COMPILE) -o main.bin $(OBJECTS)

main.hex:	main.bin
	rm -f main.hex main.eep.hex
	avr-objcopy -j .text -j .data -O ihex main.bin main.hex
	avr-size main.bin
disasm:	main.bin
	avr-objdump -d main.bin

cpp:
	$(COMPILE) -E main.c
