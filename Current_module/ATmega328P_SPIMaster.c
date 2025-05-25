#include <avr/io.h>
#include "ATmega328P_SPIMaster.h"


#define SPI_Wait()		while (!(SPSR & (1 << SPIF)));		//wait until transfer is completed


void SPI_Init(uint8_t CLK_prescaler, uint8_t first_bit, uint8_t SPI_mode)
{
	SPCR = 0x00;		//set register to default state
	SPSR = 0x00;		//set register to default state
	
	switch (CLK_prescaler)
	{
		case 2: SPSR |= (1 << SPI2X); break;
		case 4: break;
		case 8: SPCR |= (1 << SPR0); SPSR |= (1 << SPI2X); break;
		case 16: SPCR |= (1 << SPR0); break;
		case 32: SPCR |= (1 << SPR1); SPSR |= (1 << SPI2X); break;
		case 64: SPCR |= (1 << SPR1); break;
		case 128: SPCR |= (1 << SPR0) | (1 << SPR1); break;
		default: SPCR |= (1 << SPR0); SPCR |= (1 << SPR1); break;	//128
	}

	SPCR |= (SPI_mode << CPHA);		//set CPOL and CPHA bits
	SPCR |= (first_bit << DORD);	//set correct data order
	
	//set I/O pins
	DDRB |= (1 << DDB5);		//SCLK
	DDRB &= ~(1 << DDB4);		//MISO
	DDRB |= (1 << DDB3);		//MOSI
	
	SPCR |= (1 << MSTR);		//master select
	SPCR |= (1 << SPE);			//enable SPI
}


uint8_t SPI_Transmit8(uint8_t data)
{
	SPDR = data;	//put data into buffer
	SPI_Wait();		//wait until transmission is completed
	return SPDR;
}


uint16_t SPI_Transmit16(uint16_t data)
{
	uint16_t data_r;
	
	if (SPCR & (1 << DORD))
	{
		data_r = SPI_Transmit8(data);
		data_r |= (SPI_Transmit8(data >> 8) << 8);
	}
	else
	{
		data_r = (SPI_Transmit8(data >> 8) << 8);
		data_r |= SPI_Transmit8(data);
	}

	return data_r;
}


uint32_t SPI_Transmit24(uint32_t data)
{
	uint32_t data_r;
	
	if (SPCR & (1 << DORD))
	{
		data_r = SPI_Transmit8(data);
		data_r |= (SPI_Transmit8(data >> 8) << 8);
		data_r |= (SPI_Transmit8(data >> 16) << 16);
	}
	else
	{
		data_r = (SPI_Transmit8(data >> 16) << 16);
		data_r |= (SPI_Transmit8(data >> 8) << 8);
		data_r |= SPI_Transmit8(data);
	}

	return data_r;
}


uint32_t SPI_Transmit32(uint32_t data)
{
	uint32_t data_r;
	
	if (SPCR & (1 << DORD))
	{
		data_r = SPI_Transmit8(data);
		data_r |= (SPI_Transmit8(data >> 8) << 8);
		data_r |= (SPI_Transmit8(data >> 16) << 16);
		data_r |= (SPI_Transmit8(data >> 24) << 24);
	}
	else
	{
		data_r = (SPI_Transmit8(data >> 24) << 24);
		data_r |= (SPI_Transmit8(data >> 16) << 16);
		data_r |= (SPI_Transmit8(data >> 8) << 8);
		data_r |= SPI_Transmit8(data);
	}

	return data_r;
}


uint8_t SPI_Read8(void)
{
	return SPI_Transmit8(0x00);
}


uint16_t SPI_Read16(void)
{
	return SPI_Transmit16(0x0000);
}


uint32_t SPI_Read24(void)
{
	return SPI_Transmit24(0x00000000);
}


uint32_t SPI_Read32(void)
{
	return SPI_Transmit32(0x00000000);
}
