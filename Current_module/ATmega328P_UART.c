#include <avr/io.h>
#include <avr/interrupt.h>
#include "ATmega328P_UART.h"


#define RX_BUFFER_SIZE	128


volatile static uint8_t RX_buffer[RX_BUFFER_SIZE] = {0};	//buffer for received data, in the beginning all positions are 0
volatile static uint8_t RX_counter = 0;						//number of non read bytes in buffer
volatile static uint8_t RX_write_position = 0;				//next free position in buffer
volatile static uint8_t RX_read_position = 0;				//variable for last read position in buffer


ISR (USART_RX_vect)
{
	RX_buffer[RX_write_position] = UDR0;					//put new byte into buffer
	RX_write_position++;
	RX_counter++;
	
	if (RX_write_position >= RX_BUFFER_SIZE)
	{
		RX_write_position = 0;
	}
}


void UART_Init(uint32_t baud_rate, unsigned long CLK_FREQ)
{
	UCSR0A = 0x00;									//set register to default value
	
	if (baud_rate == 2000000)						//in case of max baud rate (2Mbps) double the speed
	{
		baud_rate = 1000000;
		UCSR0A |= (1 << U2X0);
	}
	baud_rate = CLK_FREQ / (16 * baud_rate) - 1;	//calculate UBRR0 value
	UBRR0H = (baud_rate & 0x0F00) >> 8;				//upper 4 bits
	UBRR0L = (baud_rate & 0x00FF);					//lower 8 bits
	
	UCSR0B = 0x00;									//set register to default state
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);			//enable receiver and transmitter
	UCSR0B |= (1 << RXCIE0);						//RX complete interrupt enable
	
	UCSR0C = 0x00;									//asynchronous USART, no parity, 1 stop bit
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);		//8 bit data transmission
	
	DDRD &= ~(1 << DDD0);		//RX as input
	DDRD |= (1 << DDD1);		//TX as output
}


void UART_SendByte(uint8_t c)
{
	while (!(UCSR0A & (1 << UDRE0)));				//wait until data buffer is empty and ready for transmission
	UDR0 = c;										//store data to register
}


void UART_SendString(uint8_t *string)
{
	uint16_t i = 0;
	
	do
	{
		UART_SendByte(string[i]);
		i++;
	} while (string[i] != '\0');
}


uint8_t UART_ReadByte(void)
{
	uint8_t data = 0;
	
	data = RX_buffer[RX_read_position];				//get data from position in buffer
	RX_read_position++;
	RX_counter--;
	
	if (RX_read_position >= RX_BUFFER_SIZE)
	{
		RX_read_position = 0;
	}
	
	return data;
}


void UART_ReadLine(uint8_t *string)
{
	uint8_t byte;
	uint8_t i = 0;
	
	do
	{
		while (UART_AvailableBytes() == 0);		//wait until something is available in RX buffer
		byte = UART_ReadByte();
		string[i++] = byte;
	} while(byte != '\n');
	
	string[i-1] = '\0';		//i-1 to remove '\n' from string
}


uint8_t UART_AvailableBytes(void)
{
	return RX_counter;
}


void UART_ClearRXBuffer(void)
{
	for (uint8_t i = 0; i < RX_BUFFER_SIZE; i++)
	{
		RX_buffer[i] = 0;		//set content of each byte in RX buffer to 0
	}
	
	RX_counter = 0;
	RX_write_position = 0;
	RX_read_position = 0;
}
