//===============================================================================
//GPIO pins library for STM32F429ZI (should work on multiple STM32F4xx platforms)
//by Martin Praznovsky, 2024
//===============================================================================


#include "stm32f429xx.h"


#ifndef STM32F429ZI_GPIOPINS_H_
#define STM32F429ZI_GPIOPINS_H_

#define INPUT									0
#define OUTPUT								1
#define ALTERNATE_FUNCTION		2
#define ANALOG_MODE						3

#define PUSH_PULL							0
#define OPEN_DRAIN						1

#define LOW_SPEED							0
#define MEDIUM_SPEED					1
#define HIGH_SPEED						2
#define VERY_HIGH_SPEED				3

#define NO_PULL								0
#define PULL_UP								1
#define PULL_DOWN							2

#define LOW										0
#define HIGH									1

#define RISING_EDGE						0
#define FALLING_EDGE					1
#define BOTH_EDGES						2

/**
* @brief - init GPIO pin as input or output
* @param GPIOx - GPIO port
* @param pin - number of pin in corresponding port (0 - 15)
* @param mode - INPUT or OUTPUT
* @param type - PUSH_PULL or OPEN_DRAIN, when using pin as INPUT, this parameter does not affect pin
* @param speed - LOW_SPEED, MEDIUM_SPEED, HIGH_SPEED, VERY_HIGH_SPEED
* @param pull - NO_PULL, PULL_UP, PULL_DOWN, when using pin as OUTPUT, this parameter does not affect pin
* @returns - nothing
*/
void GPIO_InitPin(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t mode, uint8_t type, uint8_t speed, uint8_t pull);

/**
* @brief - write logic 0 or 1 to the pin
* @param GPIOx - GPIO port
* @param pin - number of pin in corresponding port (0 - 15)
* @param value - LOW or HIGH
* @returns - nothing
*/
void GPIO_WritePin(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t value);

/**
* @brief - get state of pin, function returns 0 or 1
* @param GPIOx - GPIO port
* @param pin - number of pin in corresponding port (0 - 15)
* @returns - status of the pin (0 or 1)
*/
uint8_t GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint8_t pin);

/**
* @brief - alternate function selection
* @param GPIOx - GPIO port
* @param pin - number of pin in corresponding port (0 - 15)
* @param function - alternate function (0 - 15)
* @returns - nothing
*/
void GPIO_AlternateFunction(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t function);

/**
* @brief - configure external interrupt on pin
* it is necesarry to create interrupt service routine in main code, e.g. void EXTI0_IRQnHandler(void) {}
* in interrupt service routine program must should check EXTI-PR register for pending interrupt on corresponding bit
* pending interrupt in EXTI-PR must be cleared by programming corresponding bit to 1
* @param GPIOx - GPIO port
* @param pin - number of pin in corresponding port (0 - 15)
* @param pull - selection of resistor (NO_PULL, PULL_UP, PULL_DOWN)
* @param edge - interrupt trigger on RISING_EDGE, FALLING_EDGE or BOTH_EDGES
* @param priority - priority of interrupt
* @returns - nothing
*/
void GPIO_ConfigureExternalInterrupt(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t pull, uint8_t edge, uint8_t priority);

#endif
