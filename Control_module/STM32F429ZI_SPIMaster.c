#include "STM32F429ZI_SPIMaster.h"


void SPI1_Init(uint8_t APB2_prescaler, uint8_t first_bit, uint8_t SPI_mode)
{
	RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;				//reset SPI1 register
	RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;				//clear reset of SPI1 registers
	
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;						//enable SPI1 clock
	
	if (SPI_mode == SPI_MODE_0) {SPI1->CR1 |= (0 << 1) | (0 << 0);}
	else if (SPI_mode == SPI_MODE_0) {SPI1->CR1 |= (0 << 1) | (1 << 0);}
	else if (SPI_mode == SPI_MODE_0) {SPI1->CR1 |= (1 << 1) | (0 << 0);}
	else if (SPI_mode == SPI_MODE_0) {SPI1->CR1 |= (1 << 1) | (1 << 0);}
	
	SPI1->CR1 |= SPI_CR1_MSTR;										//configure SPI1 as master
	SPI1->CR1 |= (first_bit << 7);								//set LSBFIRST or MSBFIRST
	SPI1->CR1 |= (APB2_prescaler << 3);						//set SPI speed prescaller
	SPI1->CR1 |= SPI_CR1_SSM;											//software slave management, NSS (slave select) pin remains free for other applications
	SPI1->CR1 |= SPI_CR1_SSI;											//internal slave select, does not require external NSS signal to enable communication
	
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;									//IO port A clock enable
	GPIOA->MODER &= 0xFFFF03FF;														//clear MODER7, MODER6, MODER5
	GPIOA->MODER |= (2 << 14) | (2 << 12) | (2 << 10);		//alternate function on PA7, PA6, PA5
	GPIOA->OSPEEDR &= 0xFFFF03FF;													//clear OSPEEDR7, OSPEEDR6, OSPEEDR5
	GPIOA->OSPEEDR |= (3 << 14) | (3 << 12) | (3 << 10);	//very high speed on on PA7, PA6, PA5
	GPIOA->AFR[0] &= 0x000FFFFF;													//clear AFRL7, AFRL6, AFRL5
	GPIOA->AFR[0] |= (5 << 28) | (5 << 24) | (5 << 20);		//alternate function AF5 (SPI1) on pin PA7, PA6, PA5
	
	SPI1->CR1 |= SPI_CR1_SPE;			//SPI enable
}


uint8_t SPI1_Transmit8(uint8_t data)
{
	while(!((SPI1->SR) & SPI_SR_TXE));		//wait TXE bit to set (transmitt buffer empty)
	SPI1->DR = data;											//load data into data register
	while(!((SPI1->SR) & SPI_SR_RXNE));		//wait RXE bit to set (receive buffer not empty)
	
	return SPI1->DR;											//read data register
}


uint8_t SPI1_Transmit16(uint16_t data)
{
	uint16_t data_r;
	
	if (SPI1->CR1 & SPI_CR1_LSBFIRST)
	{
		data_r = SPI1_Transmit8(data);
		data_r |= (SPI1_Transmit8(data >> 8) << 8);
	}
	else
	{
		data_r = (SPI1_Transmit8(data >> 8) << 8);
		data_r |= SPI1_Transmit8(data);
	}
	
	return data_r;
}


uint8_t SPI1_Transmit24(uint32_t data)
{
	uint32_t data_r;
	
	if (SPI1->CR1 & SPI_CR1_LSBFIRST)
	{
		data_r = SPI1_Transmit8(data);
		data_r |= (SPI1_Transmit8(data >> 8) << 8);
		data_r |= (SPI1_Transmit8(data >> 16) << 16);
	}
	else
	{
		data_r = (SPI1_Transmit8(data >> 16) << 16);
		data_r |= (SPI1_Transmit8(data >> 8) << 8);
		data_r |= SPI1_Transmit8(data);
	}
	
	return data_r;
}


uint8_t SPI1_Transmit32(uint32_t data)
{
	uint32_t data_r;
	
	if (SPI1->CR1 & SPI_CR1_LSBFIRST)
	{
		data_r = SPI1_Transmit8(data);
		data_r |= (SPI1_Transmit8(data >> 8) << 8);
		data_r |= (SPI1_Transmit8(data >> 16) << 16);
		data_r |= (SPI1_Transmit8(data >> 24) << 24);
	}
	else
	{
		data_r = (SPI1_Transmit8(data >> 24) << 24);
		data_r |= (SPI1_Transmit8(data >> 16) << 16);
		data_r |= (SPI1_Transmit8(data >> 8) << 8);
		data_r |= SPI1_Transmit8(data);
	}
	
	return data_r;
}


uint8_t SPI1_Read8()
{
	return SPI1_Transmit8(0x00);
}

uint8_t SPI1_Read16()
{
	return SPI1_Transmit16(0x0000);
}

uint8_t SPI1_Read24()
{
	return SPI1_Transmit24(0x00000000);
}

uint8_t SPI1_Read32()
{
	return SPI1_Transmit32(0x00000000);
}
