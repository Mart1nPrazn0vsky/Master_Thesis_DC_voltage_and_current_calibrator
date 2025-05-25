//===========================================
//UART library for ATMEGA328P microcontroller
//by Martin Praznovsky, 2023
//===========================================


#ifndef ATMEGA328P_UART_H_
#define ATMEGA328P_UART_H_

/**
* @brief - initialization of UART, "baud_rate" is number defined by user
* @param baud_rate - desired speed of UART communication
* @param CLK_FREQ - frequency of MCU clock
* @returns - nothing
*/
void UART_Init(uint32_t baud_rate, unsigned long CLK_FREQ);

/**
* @brief - send byte of data
* @param c - byte to be send
* @returns - nothing
*/
void UART_SendByte(uint8_t c);

/**
* @brief - send string of data, string must be finished with '\0'
* @param string - string of characters to be send
* @returns - nothing
*/
void UART_SendString(uint8_t *string);

/**
* @brief - read byte from UART receiver buffer
* @returns - received byte
*/
uint8_t UART_ReadByte(void);

/**
* @brief - read line of received data ending with '\n', '\n' is removed from string, string ends with '\0'
* @param string - array to which received line is saved
* @returns - nothing
*/
void UART_ReadLine(uint8_t *string);

/**
* @brief - get number of available received bytes
* @returns - number of bytes available in receiver buffer
*/
uint8_t UART_AvailableBytes(void);

/**
* @brief - delete content of RX buffer, set counters to 0
* @returns - nothing
*/
void UART_ClearRXBuffer(void);

#endif /* ATMEGA328P_UART_H_ */
