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
#include "Calibrator_calibration_constants.h"



#ifndef CCB_MODULE_H_
#define CCB_MODULE_H_

#define CCB_OUTPUT_OFF				0
#define CCB_OUTPUT_ON					1
#define CCB_AUTORANGE_OFF			0
#define CCB_AUTORANGE_ON			1
#define CCB_DITHERING_OFF			0
#define CCB_DITHERING_ON			1

#define NO_ERROR							0
#define ERROR_COMMUNICATION		1
#define ERROR_VOLT_RANGE			2
#define ERROR_FREQ_RANGE			3

typedef struct
{
	double current;
	uint8_t range;
	uint8_t output_state;
	uint8_t autorange_state;
	uint8_t dithering_state;
} CCB_module_state;

uint8_t CCB_Init(UART *UART_handle);

uint8_t CCB_WriteToRegister(uint8_t reg, uint32_t data);

uint8_t CCB_ReadAllRegisters(void);

uint8_t CCB_CompareRegisters(void);

uint8_t CCB_SetRange(uint8_t range);

void CCB_AutorangeON(void);

void CCB_AutorangeOFF(void);

uint8_t CCB_OutputON(void);

uint8_t CCB_OutputOFF(void);

uint8_t CCB_DitheringON(void);

uint8_t CCB_DitheringOFF(void);

uint8_t CCB_CheckRange(double voltage);

uint8_t CCB_Autorange(double voltage);

uint8_t CCB_SetCurrent(double current);

uint32_t CCB_GetVoltageCode(double current);

uint8_t CCB_TurnOFFModule(void);

double CCB_GetCurrent(void);

uint8_t CCB_GetRange(void);

uint8_t CCB_GetMode(void);

uint8_t CCB_GetOutputState(void);

uint8_t CCB_GetAutorangeState(void);

void CCB_GetModuleState(CCB_module_state *handle);

#endif
