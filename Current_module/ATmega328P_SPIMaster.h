//==========================================
//SPI library for ATMEGA328P microcontroller
//by Martin Praznovsky, 2023
//==========================================


#ifndef ATMEGA328P_SPI_H_
#define ATMEGA328P_SPI_H_

#define MSB_FIRST		0
#define LSB_FIRST		1

#define SPI_MODE_0		0
#define SPI_MODE_1		1
#define SPI_MODE_2		2
#define SPI_MODE_3		3

#define MISO_NO_PULL	0
#define MISO_PULL_UP	1

/**
* @brief - initialization of SPI, "prescaler" of system clock is set by user, "bit" is MSB_FIRST/LSB_FIRST
* @param CLK_prescaler - prescaler of MCU clock, thus setting SPI speed of communication (2, 4, 8, 16, 32, 64, 128)
* @param first_bit - first bit to be send, MSB_FIRST or LSB_FIRST
* @param SPI_mode - SPI mode depending on CPOL and CPHA
* SPI_MODE_0: CPOL = 0, CPHA = 0, CLK idle state = low, data sampled on rising and shifted out on falling edge
* SPI_MODE_1: CPOL = 0, CPHA = 1, CLK idle state = low, data sampled on falling and shifted out on rising edge
* SPI_MODE_2: CPOL = 1, CPHA = 1, CLK idle state = high, data sampled on falling and shifted out on rising edge
* SPI_MODE_3: CPOL = 1, CPHA = 0, CLK idle state = high, data sampled on rising and shifted out on falling edge
* @returns - nothing
*/
void SPI_Init(uint8_t CLK_prescaler, uint8_t first_bit, uint8_t SPI_mode);

/**
* @brief - send 8 bits of data
* @param data - data to be send
* @returns - 8 bits of received data
*/
uint8_t SPI_Transmit8(uint8_t data);

/**
* @brief - send 16 bits of data
* @param data - data to be send
* @returns - 16 bits of received data
*/
uint16_t SPI_Transmit16(uint16_t data);

/**
* @brief - send 24 bits of data
* @param data - data to be send
* @returns - 24 bits of received data
*/
uint32_t SPI_Transmit24(uint32_t data);

/**
* @brief - send 32 bits of data
* @param data - data to be send
* @returns - 32 bits of received data
*/
uint32_t SPI_Transmit32(uint32_t data);

/**
* @brief - read 8 bits of data
* @returns - 8 bits of received data
*/
uint8_t SPI_Read8(void);

/**
* @brief - read 16 bits of data
* @returns - 16 bits of received data
*/
uint16_t SPI_Read16(void);

/**
* @brief - read 24 bits of data
* @returns - 24 bits of received data
*/
uint32_t SPI_Read24(void);

/**
* @brief - read 32 bits of data
* @returns - 32 bits of received data
*/
uint32_t SPI_Read32(void);

#endif /* ATMEGA328P_SPI_H_ */
