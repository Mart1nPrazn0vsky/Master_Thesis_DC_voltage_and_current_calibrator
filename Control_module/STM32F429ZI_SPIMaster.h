//================================================================================
//SPI master library for STM32F429ZI (should work on multiple STM32F4xx platforms)
//by Martin Praznovsky, 2024
//================================================================================


#include "stm32f429xx.h"


#ifndef STM32F429ZI_SPIMASTER_H_
#define STM32F429ZI_SPIMASTER_H_

#define MSBFIRST					0
#define LSBFIRST					1

#define SPI_MODE_0				0
#define SPI_MODE_1				1
#define SPI_MODE_2				2
#define SPI_MODE_3				3

#define PRESCLALER_2			0
#define PRESCLALER_4			1
#define PRESCLALER_8			2
#define PRESCLALER_16			3
#define PRESCLALER_32			4
#define PRESCLALER_64			5
#define PRESCLALER_128		6
#define PRESCLALER_256		7

void SPI1_Init(uint8_t APB2_prescaler, uint8_t first_bit, uint8_t SPI_mode);

uint8_t SPI1_Transmit8(uint8_t data);

uint8_t SPI1_Transmit16(uint16_t data);

uint8_t SPI1_Transmit24(uint32_t data);

uint8_t SPI1_Transmit32(uint32_t data);

uint8_t SPI1_Read8();

uint8_t SPI1_Read16();

uint8_t SPI1_Read24();

uint8_t SPI1_Read32();

#endif
