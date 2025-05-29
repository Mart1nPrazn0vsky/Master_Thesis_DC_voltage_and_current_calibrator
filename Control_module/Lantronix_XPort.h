//===========================================================================================
//Library for simple initialization and control of Lantronix XPort Ethernet to UART interface
//by Martin Praznovsky, 2025
//===========================================================================================

#include "stm32f429xx.h"
#include "string.h"
#include "STM32F429ZI_Delay.h"
#include "STM32F429ZI_GPIOPins.h"
#include "STM32F429ZI_UART.h"
#include "stdio.h"


#ifndef LANTRONIXXPORT_H_
#define LANTRONIXXPORT_H_


/**
* @brief - init Lantronix XPort device, corresponding UART must be already initialized with speed 9600
* @param UART_handle - UART type handle of UART line to which is Lantronix XPort connected
* @param speed - desired speed of Lantronix XPort serial interface, if speed is different than 9600, after calling this function, UART line must be reinitialized to correct speed
* @param reset_port - GPIO port of reset pin
* @param reset_pin - number of reset pin
* @returns - nothing
*/
void Lantronix_XPort_Init(UART *UART_handle, uint32_t speed, GPIO_TypeDef *reset_port, uint8_t reset_pin);

/**
* @brief - reset Lantronix XPort device
* @returns - nothing
*/
void Lantronix_XPort_Reset(void);

/**
* @brief - get  IP address, gateway, subnet mask and DNS of Ethernet connection
* @param string_IP - IP address will be saved into this string
* @param string_gateway - gateway will be saved into this string
* @param string_mask - subnet mask will be saved into this string
* @param string_DNS - DNS will be saved into this string
* @returns - nothing
*/
void Lantronix_XPort_GetIPAddress(uint8_t *string_IP, uint8_t *string_gateway, uint8_t *string_mask, uint8_t *string_DNS);

#endif
