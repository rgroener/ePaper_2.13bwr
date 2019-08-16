
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

#define START_PARTIAL_MODE epap_send_command(0x91)
#define START_NORMAL_MODE epap_send_command(0x92)

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
void epap_clear_white(void);
void epap_init(void);

void epap_write_data(uint16_t lenght, uint8_t data)
{
	uint16_t xx=0;
	//Display transmission 1 (b/w pixel)
	epap_send_command(0x10);
	for(xx=0; xx<lenght;xx++)
	{
		//1=white 0=black
		epap_send_data(data);
	}
	//end sendet data
	epap_send_command(0x11);	
	//Display transmission 2 (red pixel)
	epap_send_command(0x13);
	for(xx=0; xx<lenght;xx++)
	{
		//0=red  1=transp
		epap_send_data(0xFF);
		
	}
	epap_send_command(0x12);
	while(BUSY_LOW);
}
void epap_set_part_window(uint8_t hor_start, uint8_t hor_stop, uint8_t vert_start, uint8_t vert_stop, uint8_t scan)
{
	//partial window
	//bit shifting according datasheet 
	//(32) Partial Window(PTL)
	epap_send_command(0x90);
	epap_send_data(hor_start<<3);
	epap_send_data(hor_stop<<3);
	epap_send_data(vert_start>>8);
	epap_send_data(vert_start);
	epap_send_data(vert_stop>>8);
	epap_send_data(vert_stop);
	if(scan==1)
	{
		epap_send_data(0x01);//Gates scan only outside of the window (default)
	}else epap_send_data(0x00);//Gates scan only inside of the window
}

int main(void)
{
	DDRD |= (1<<PD1);//output led

	spi_init();
    sei();
	epap_init();
	START_NORMAL_MODE;
	epap_clear_white();
	
	START_PARTIAL_MODE;
	//enter partial mode
	epap_set_part_window(0,2,10,200,0);
		
	epap_write_data(200, 0x00);

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

void epap_init(void)
{
	//Software reset
	epap_send_data(0x12);
	//panel setting
	epap_send_command(0x00);
	epap_send_data(0x13);
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
	while(BUSY_LOW);
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
	//epap_clear_white();
};
void epap_clear_white(void)
{
	uint16_t xx=0;
	//Display transmission 1 (b/w pixel)
	epap_send_command(0x10);
	for(xx=0; xx<2756;xx++)
	{
		epap_send_data(0xFF);
	}
	//end sendet data
	epap_send_command(0x11);	
	//Display transmission 2 (red pixel)
	epap_send_command(0x13);
	for(xx=0; xx<2756;xx++)
	{
		epap_send_data(0xFF);
		
	}
	
	epap_send_command(0x12);
	while(BUSY_LOW);
}

