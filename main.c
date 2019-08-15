
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
 
#define ACK 0x7E

#define BUSY_LOW !(PIND & (1<<PD0))		//check if display is busy
#define CS_LOW PORTC &= ~(1<<PC1)		//set cs low
#define CS_HIGH PORTC |= (1<<PC1)		//set cs high

#define SEND_DATA PORTC |= (1<<PC0)		//send data to display
#define SEND_COMMAND PORTC &= ~(1<<PC0)	//send command to display

#define LED_EIN PORTD |= (1<<PD1)
#define LED_AUS PORTD &= ~(1<<PD1)
void spi_init(void);
unsigned char epap_send_data(unsigned char data);
unsigned char epap_send_command(unsigned char data);

const uint8_t LUT_DATA_full[] PROGMEM =
{
  0x80, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, //LUT0: BB:     VS 0 ~7
  0x10, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, //LUT1: BW:     VS 0 ~7
  0x80, 0x60, 0x40, 0x00, 0x00, 0x00, 0x00, //LUT2: WB:     VS 0 ~7
  0x10, 0x60, 0x20, 0x00, 0x00, 0x00, 0x00, //LUT3: WW:     VS 0 ~7
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //LUT4: VCOM:   VS 0 ~7
  0x03, 0x03, 0x00, 0x00, 0x02, // TP0 A~D RP0
  0x09, 0x09, 0x00, 0x00, 0x02, // TP1 A~D RP1
  0x03, 0x03, 0x00, 0x00, 0x02, // TP2 A~D RP2
  0x00, 0x00, 0x00, 0x00, 0x00, // TP3 A~D RP3
  0x00, 0x00, 0x00, 0x00, 0x00, // TP4 A~D RP4
  0x00, 0x00, 0x00, 0x00, 0x00, // TP5 A~D RP5
  0x00, 0x00, 0x00, 0x00, 0x00, // TP6 A~D RP6
};

uint16_t x;

int main(void)
{
	
	DDRD |= (1<<PD1);//output led
	
	//_delay_ms(400);
	
	
	spi_init();
    sei();
 
	while(BUSY_LOW);
	//Software reset
	epap_send_data(0x12);
	
	//panel setting
	epap_send_command(0x00);
	epap_send_data(0x0F);

	//booster soft start
	epap_send_command(0x06);
	epap_send_data(0x17);
	epap_send_data(0x17);
	epap_send_data(0x17);
	//Power setting
	epap_send_command(0x01);
	epap_send_data(0x03);
	epap_send_data(0x00);
	epap_send_data(0x2b);
	epap_send_data(0x2b);
	epap_send_data(0x09);

	//Power on
	epap_send_command(0x04);
	
	
	
	while(BUSY_LOW)
	{
		_delay_ms(200);
		LED_EIN;
		
		
	}
	_delay_ms(200);
	epap_send_command(0x12);
	while(BUSY_LOW)
	{
		_delay_ms(200);
		LED_EIN;
		
		
	}
	//Panel setting
	epap_send_command(0x00);
	epap_send_data(0xaf);
	
	//Pll control
	epap_send_command(0x03);
	epap_send_data(0x3a);
	//Resolution setting
	epap_send_command(0x61);
	epap_send_data(0x68);
	epap_send_data(0x00);
	epap_send_data(0xD4);
	//VCM DC setting
	epap_send_command(0x82);
	epap_send_data(0x12);
	//Vcom and data interval setting
	epap_send_command(0x50);
	epap_send_data(0x77);
	
	//Display transmission 1
	epap_send_command(0x10);
	for(x=0; x<1000;x++)
	{
		epap_send_data(0x0F);
		
	}
		
	//Display transmission 2
	epap_send_command(0x13);
	for(x=0; x<1000;x++)
	{
		epap_send_data(0xFF);
		
	}
	epap_send_command(0x12);
	while(BUSY_LOW)
	{
		_delay_ms(200);
		LED_EIN;
		
		
	}
	
	LED_EIN;
	
	
	while(1)
	{ 
			
	} //end while
}//end of main

// Initialize SPI Master Device (with SPI interrupt)
void spi_init(void)
{
    // Set MOSI, SCK as Output
    DDRB=(1<<5)|(1<<3);
	DDRC = ((1<<PC0) | (1<<PC1));//Output
	PORTC = ((1<<PC0) | (1<<PC1));//set high
	DDRB &= ~(1<<PB4);//Input (Miso)
	DDRD &= ~(1<<PD0);//Input
	//PORTD |= (1<<PD0);//epaper-BUSY; activate pullup
    // Enable SPI, Set as Master
    // Prescaler: Fosc/16, Enable Interrupts
    //The MOSI, SCK pins are as per ATMega8
    // Enable SPI as master, set clock rate fck/2
    SPCR = (1<<SPE) | (1<<MSTR);
    SPSR = (1<<SPI2X);
 
    // Enable Global Interrupts
    sei();
}
void spi_close()
{
	SPCR = 0x00;//clear spi enable bit
}

//Function to send and receive data for both master and slave
unsigned char epap_send_data(unsigned char data)
{
	CS_LOW;
	SEND_DATA;
    // Load data into the buffer
    SPDR = data;
    //Wait until transmission complete
    while(!(SPSR & 0x80 ));
	CS_HIGH;
    // Return received data
    return(SPDR);
    
}
//Function to send and receive data for both master and slave
unsigned char epap_send_command(unsigned char data)
{
	CS_LOW;
	SEND_COMMAND;
    // Load data into the buffer
    SPDR = data;
    //Wait until transmission complete
    while(!(SPSR & 0x80 ));
	CS_HIGH;
    // Return received data
    return(SPDR);
    
}


