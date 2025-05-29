//==================================================
//Library for Calibrator Current Board (CCB) control
//by Martin Praznovsky, 2025
//==================================================

#include "stm32f429xx.h"
#include "stdio.h"
#include "string.h"
#include "STM32F429ZI_Delay.h"
#include "STM32F429ZI_UART.h"
#include "Calibrator_utils.h"
#include "Calibrator_module.h"
#include "Calibrator_calibration_constants.h"


#ifndef CCB_MODULE_H_
#define CCB_MODULE_H_

#define CCB_OUTPUT_OFF				0
#define CCB_OUTPUT_ON					1
#define CCB_AUTORANGE_OFF			0
#define CCB_AUTORANGE_ON			1
#define CCB_DITHERING_OFF			0
#define CCB_DITHERING_ON			1

#define LED_4R 								14		//not active module
#define LED_4G 								13		//active module
#define LED_3R 								12		//output off
#define LED_3G 								11		//output on
#define LED_2R 								10
#define LED_2G 								9
#define LED_1R 								8
#define LED_1G 								7

typedef struct
{
	double current;
	uint8_t range;
	uint8_t output_state;
	uint8_t autorange_state;
	uint8_t dithering_state;
} CCB_module_state;


uint8_t CCB_Init(UART *UART_handle);

uint8_t CCB_TurnOFFModule(void);

uint8_t CCB_SetRange(uint8_t range);

uint8_t CCB_CheckRange(double current);

uint8_t CCB_Autorange(double current);

void CCB_AutorangeON(void);

void CCB_AutorangeOFF(void);

uint8_t CCB_OutputON(void);

uint8_t CCB_OutputOFF(void);

uint8_t CCB_DitheringON(void);

uint8_t CCB_DitheringOFF(void);

uint8_t CCB_SetCurrent(double current);

uint32_t CCB_GetVoltageCode(double current);

uint8_t CCB_SetLED(uint8_t led, uint8_t value);

double CCB_GetCurrent(void);

uint8_t CCB_GetRange(void);

uint8_t CCB_GetAutorangeState(void);

uint8_t CCB_GetOutputState(void);

uint8_t CCB_GetDitheringState(void);

uint8_t CCB_GetMode(void);

//debugging only
uint8_t CCB_PrintRegisters(UART *UART_USB);

#endif
