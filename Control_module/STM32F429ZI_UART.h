//==========================================================================
//UART library for STM32F429ZI (should work on multiple STM32F4xx platforms)
//by Martin Praznovsky, 2023
//==========================================================================


#include "stm32f429xx.h"
#include <stdint.h>
#include "STM32F429ZI_GPIOPins.h"


#ifndef STM32F429ZI_UART_H_
#define STM32F429ZI_UART_H_

#define UART1_RX_BUFFER_SIZE	128
#define UART2_RX_BUFFER_SIZE	128
#define UART3_RX_BUFFER_SIZE	128
#define UART4_RX_BUFFER_SIZE	128
#define UART5_RX_BUFFER_SIZE	128
#define UART6_RX_BUFFER_SIZE	128
#define UART7_RX_BUFFER_SIZE	128
#define UART8_RX_BUFFER_SIZE	128

typedef struct
{
	USART_TypeDef* UARTx;
	uint8_t *UART_RX_buffer;
	uint8_t UART_RX_buffer_size;
	uint8_t *UART_RX_counter;
	uint8_t *UART_RX_write_pos;
	uint8_t *UART_RX_read_pos;
} UART;

/**
* @brief - init UARTx serial line
* @param UARTx - corresponding USART_TypeDef of UART line (USART1, USART2, USART3, etc.)
* @param baud_rate - speed of UART line
* @param CLK_FREQ - frequency of peripheral clock (APB1 clock for USART2, USART3, USART4, USART5, USART7, USART8, APB2 clock for USART1, USART6)
* @param priority - priority of interrupt assigned to UART receiver
* @param TX_port - name of GPIO port of TX pin
* @param TX_pin - number of TX pin
* @param RX_port - name of GPIO port of RX pin
* @param RX_pin - name of TX pin
* @returns - UART data type of corresponding UART line
*/
UART *UART_Init(USART_TypeDef *UARTx, uint32_t baud_rate, uint32_t CLK_FREQ, uint8_t priority, GPIO_TypeDef *TX_port, uint8_t TX_pin, GPIO_TypeDef *RX_port, uint8_t RX_pin);

/**
* @brief - send one byte of data
* @param UARTx - UART which is going to be used
* @param byte - 8 bits of data
* @returns - nothing
*/
void UART_SendByte(UART *UARTx, uint8_t byte);

/**
* @brief - send string of bytes, string must be finished with '\0', '\n' is not send automatically
* @param UARTx - UART which is going to be used
* @param string - pointer to string to be send
* @returns - nothing
*/
void UART_SendString(UART *UARTx, uint8_t *string);

/**
* @brief - read one byte from RX buffer
* @param UARTx - UART which is going to be used
* @returns - oldest received byte in buffer
*/
uint8_t UART_ReadByte(UART *UARTx);

/**
* @brief - read line of received data ending with '\n' or '\r'
* @param UARTx - UART which is going to be used
* @param string - pointer to string into which is received string saved, '\n' or '\r' is removed from string
* @returns - nothing
*/
void UART_ReadLine(UART *UARTx, uint8_t *string);

/**
* @brief - get number of available received bytes in RX buffer
* @param UARTx - UART which is going to be used
* @returns - number of available received bytes in RX buffer
*/
uint8_t UART_AvailableBytes(UART *UARTx);

/**
* @brief - delete content of RX buffer, set counters to 0
* @param UARTx - UART which is going to be used
* @returns - nothing
*/
void UART_ClearRXBuffer(UART *UARTx);

#endif
