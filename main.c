
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <stdlib.h>
#include <avr/io.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
 
#define ACK 0x7E

#define BUSY_LOW !(PIND & (1<<PD0))		//check if display is busy
#define CS_LOW PORTC &= ~(1<<PC1)		//set cs low
#define SEND_DATA PORTC |= (1<<PC0)		//send data to display
#define SEND_COMMAND PORTC &= ~(1<<PC0)	//send command to display

#define LED_EIN PORTD |= (1<<PD1);
#define LED_AUS PORTD &= ~(1<<PD1);
void spi_init_master (void);
unsigned char spi_tranceiver (unsigned char data);


// SPI Transmission/reception complete ISR
ISR(SPI_STC_vect)
{
    // Code to execute
    // whenever transmission/reception
    // is complete.
}


int main(void)
{
	
	DDRD |= (1<<PD1);//output led
	
	//_delay_ms(400);
	LED_AUS;
	
	spi_init_master();
    sei();
    
    
	CS_LOW;
	 _delay_us(1); // CS setup time is ~40ns on the display
	SEND_COMMAND;
	LED_EIN;
	//Booster soft start
	//wake display
	spi_tranceiver(0x06);
	
	spi_tranceiver(0x17);
	spi_tranceiver(0x17);
	spi_tranceiver(0x17);
	
	
	//Power setting
	spi_tranceiver(0x01);
	spi_tranceiver(0x03);
	spi_tranceiver(0x00);
	spi_tranceiver(0x2b);
	spi_tranceiver(0x2b);
	spi_tranceiver(0x09);
	//Power on
	spi_tranceiver(0x04);
	
	while(BUSY_LOW);//wait as long epaper is busy
	
	//Panel setting
	spi_tranceiver(0x00);
	spi_tranceiver(0xAF);
	//PLL control
	spi_tranceiver(0x30);
	spi_tranceiver(0x3A);
	//Resolution setting
	spi_tranceiver(0x61);
	spi_tranceiver(0x68);
	spi_tranceiver(0x00);
	spi_tranceiver(0xD4);
	//VCM DC setting
	spi_tranceiver(0x82);
	spi_tranceiver(0x12);
	//Vcom and data intervall setting
	spi_tranceiver(0x50);
	spi_tranceiver(0x77);

	//start data transmission 1
	spi_tranceiver(0x10);
	SEND_DATA;
	 for (uint16_t i = 0; i < 100; ++i) {
            spi_tranceiver(0x0F);
     }
     SEND_COMMAND;
	//start data transmission 2
	spi_tranceiver(0x13);

	//Display refresh
	spi_tranceiver(0x12);
	while(BUSY_LOW);//wait as long epaper is busy
	
	
	
	LED_AUS;
	
	
	
	
	
	
	while(1)
	{ 
		
	} //end while
}//end of main

// Initialize SPI Master Device (with SPI interrupt)
void spi_init_master (void)
{
    // Set MOSI, SCK as Output
    DDRB=(1<<5)|(1<<3);
	DDRC = ((1<<PC0) | (1<<PC1));//Output
	PORTC = ((1<<PC0) | (1<<PC1));//set high
	
	DDRD &= ~(1<<PD0);//Input
	PORTD |= (1<<PD0);//epaper-BUSY; activate pullup
    // Enable SPI, Set as Master
    // Prescaler: Fosc/16, Enable Interrupts
    //The MOSI, SCK pins are as per ATMega8
    SPCR=(1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPIE);
 
    // Enable Global Interrupts
    sei();
}

//Function to send and receive data for both master and slave
unsigned char spi_tranceiver (unsigned char data)
{
    // Load data into the buffer
    SPDR = data;
 
    //Wait until transmission complete
    while(!(SPSR & (1<<SPIF) ));
 
    // Return received data
    return(SPDR);
}


