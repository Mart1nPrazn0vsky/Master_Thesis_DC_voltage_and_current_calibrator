//===========================================================================
//Delay library for STM32F429ZI (should work on multiple STM32F4xx platforms)
//by Martin Praznovsky, 2023
//===========================================================================


#include "stm32f429xx.h"
#include <stdint.h>


#ifndef STM32F429ZI_DELAY_H_
#define STM32F429ZI_DELAY_H_

/**
* @brief - init delay functionality (configure 16-bit timer 7)
* @returns - nothing
*/
void InitDelayTimer(void);

/**
* @brief - wait desired number of microseconds
* @returns - nothing
*/
void delay_us(uint16_t us);

/**
* @brief - wait desired number of miliseconds
* @returns - nothing
*/
void delay_ms(uint16_t ms);

#endif
