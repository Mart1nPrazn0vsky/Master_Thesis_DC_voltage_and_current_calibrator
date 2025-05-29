//===========================================================
//Utility functions for Calibrator Main Control Module (CMCM)
//by Martin Praznovsky, 2025
//===========================================================

#include "stm32f429xx.h"
#include "stdio.h"
#include "string.h"
#include "STM32F429ZI_UART.h"
#include "STM32F429ZI_Delay.h"
#include "Calibrator_utils.h"
#include "Calibrator_errors.h"


#ifndef CALIBRATOR_MODULE_H_
#define CALIBRATOR_MODULE_H_

/**
* @brief - write data into register and check if data were written correctly
* @param UART_handle - UART type handle of UART line to which is module connected
* @param reg_name - name of the register, starting from 'G'
* @param data - data to be written into register
* @param hex_size - size of register in hexadecimal number (16-bit register has hex_size = 4)
* @returns - NO_ERROR if operation was successful, error if writing was unsuccessful
*/
uint8_t Module_WriteToRegister(UART* UART_handle, uint8_t reg_name, uint32_t data, uint8_t hex_size);

/**
* @brief - read all registers from device and save them to defined variables
* @param UART_handle - UART type handle of UART line to which is module connected
* @returns - NO_ERROR if operation was successful, error if writing was unsuccessful
*/
uint8_t Module_ReadAllRegisters(UART* UART_handle);

/**
* @brief - read all registers from device and save them to defined variables
* @param UART_handle - UART type handle of UART line to which is module connected
* @param reg_name - name of register
* @returns - NO_ERROR if operation was successful, error if writing was unsuccessful
*/
//uint32_t Module_ReadRegister(UART* UART_handle, uint8_t reg_name);
uint8_t Module_ReadRegister(UART* UART_handle, uint8_t reg_name, uint32_t *data);

/**
* @brief - read all registers from device and save them to defined variables
* @param UART_handle - UART type handle of UART line to which is module connected
* @param reg_name - name of register
* @returns - NO_ERROR if operation was successful, error if writing was unsuccessful
*/
uint8_t Module_GetName(UART *UART_handle, uint8_t *name);

#endif
