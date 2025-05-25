//==========================================
//I2C library for ATMEGA328P microcontroller
//by Martin Praznovsky, 2023
//==========================================


#ifndef ATMEGA328P_I2C_H_
#define ATMEGA328P_I2C_H_

/**
* @brief - initialization of I2C bus
* @param bit_rate - speed of I2C bus (recommended: between 10000 and 1000000)
* @param CLK_FREQ - frequency of MCU clock
* @returns - nothing
*/
void I2C_Init(uint32_t bit_rate, unsigned long CLK_FREQ);

/**
* @brief - send 1 byte of data
* @param address - address of slave device (note that address is shifted by 1 bit do the left during transmission)
* @param data - byte of data to be send
* @returns - 0 if success, 1 in case of error
*/
uint8_t I2C_SendByte(uint8_t address, uint8_t data);

/**
* @brief - send multiple bytes of data to slave
* @param address - address of slave device (note that address is shifted by 1 bit do the left during transmission)
* @param data - bytes of data to be send
* @param length - number of bytes to be send (length of data)
* @returns - 0 if success, 1 in case of error
*/
uint8_t I2C_SendBytes(uint8_t address, uint8_t *data, uint8_t length);

/**
* @brief - read 1 byte of data
* @param address - address of slave device (note that address is shifted by 1 bit do the left during transmission)
* @param data - byte of data to be send
* @returns - 0 if success, 1 in case of error
*/
uint8_t I2C_ReadByte(uint8_t address, uint8_t data);

/**
* @brief - read multiple bytes of data from slave
* @param address - address of slave device (note that address is shifted by 1 bit do the left during transmission)
* @param data - bytes of data to be read
* @param length - number of bytes to be read (length of data)
* @returns - 0 if success, 1 in case of error
*/
uint8_t I2C_ReadBytes(uint8_t address, uint8_t *data, uint8_t length);

/**
* @brief - internal function to start I2C communication
* @param address - address of slave device (note that address is shifted by 1 bit do the left during transmission)
* @param mode - read (1) or write (0)
* @returns - 0 if success, 1 in case of error
*/
uint8_t I2C_Start(uint8_t address, uint8_t mode);

/**
* @brief - internal function to stop I2C communication
* @returns - nothing
*/
void I2C_Stop(void);

/**
* @brief - internal function for transmitting data
* @param data - byte to be send
* @returns - 0 if success, 1 in case of error
*/
uint8_t I2C_Write(uint8_t data);

/**
* @brief - internal function to read byte of data with acknowledgement at the end
* @returns - byte of received data
*/
uint8_t I2C_ReadAck(void);

/**
* @brief - internal function to read byte of data without acknowledgement at the end
* @returns - byte of received data
*/
uint8_t I2C_ReadNack(void);

#endif /* ATMEGA328P_I2C_H_ */
