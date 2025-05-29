//==================================================================================
//System clock library for STM32F429ZI (should work on multiple STM32F4xx platforms)
//by Martin Praznovsky, 2023
//==================================================================================


#include "stm32f429xx.h"


#ifndef STM32F429ZI_SYSTEMCLOCK_H_
#define STM32F429ZI_SYSTEMCLOCK_H_

#define EXT_OSCILLATOR	0
#define EXT_CLOCK				1

/**
* @brief - configure system clock to 180MHz (highest possible clock frequency on STM32F429ZI)
* APBx clocks are set to maximum possible frequency (APB1 = 45MHz, APB2 = 90MHz)
* EXT_CLOCK requires 4-26MHz clock signal on OSC_IN pin (on NUCLEO board), OSC_OUT pin is in Hi-Z
* EXT_OSCILLATOR requires 4MHz crystal or ceramic resonator between pins OSC_IN and OSC_OUT
* @param HSE_freq - frequency of high speed external clock signal
* @param HSE_source - source of HSE clock signal (EXT_OSCILLATOR or EXT_CLOCK)
*/
void SystemClockConfig(uint32_t HSE_freq, uint8_t HSE_source);

#endif
