//=======================================================
//Library for Calibrator Low Voltage Board (CLVB) control
//by Martin Praznovsky, 2025
//=======================================================

#include "stm32f429xx.h"
#include "stdio.h"
#include "string.h"
#include "STM32F429ZI_Delay.h"
#include "STM32F429ZI_UART.h"
#include "Calibrator_utils.h"
#include "Calibrator_module.h"
#include "Calibrator_calibration_constants.h"


#ifndef CLVB_MODULE_H_
#define CLVB_MODULE_H_

#define CLVB_MODE_DC					0
#define CLVB_MODE_AC					1
#define CLVB_OUTPUT_OFF				0
#define CLVB_OUTPUT_ON				1
#define CLVB_AUTORANGE_OFF		0
#define CLVB_AUTORANGE_ON			1
#define CLVB_DITHERING_OFF		0
#define CLVB_DITHERING_ON			1


typedef struct
{
	double voltage;
	double frequency;
	uint8_t range;
	uint8_t mode;
	uint8_t output_state;
	uint8_t autorange_state;
	uint8_t dithering_state;
} CLVB_module_state;


uint8_t CLVB_Init(UART *UART_handle);

uint8_t CLVB_TurnOFFModule(void);

uint8_t CLVB_SetRange(uint8_t range);

uint8_t CLVB_CheckRange(double voltage);

uint8_t CLVB_Autorange(double voltage);

void CLVB_AutorangeON(void);

void CLVB_AutorangeOFF(void);

uint8_t CLVB_OutputON(void);

uint8_t CLVB_OutputOFF(void);

uint8_t CLVB_DitheringON(void);

uint8_t CLVB_DitheringOFF(void);

uint8_t CLVB_SetVoltageDC(double voltage);

uint8_t CLVB_SetVoltageAC(double voltage, double frequency);

uint32_t CLVB_GetVoltageCode(double voltage);

uint8_t CLVB_CheckFrequency(double frequency);

uint8_t CLVB_SetFrequency(double frequency);

uint8_t CLVB_SetLED(uint8_t led, uint8_t value);

double CLVB_GetVoltage(void);

double CLVB_GetFrequency(void);

uint8_t CLVB_GetRange(void);

uint8_t CLVB_GetAutorangeState(void);

uint8_t CLVB_GetOutputState(void);

uint8_t CLVB_GetDitheringState(void);

uint8_t CLVB_GetMode(void);

//debugging only
uint8_t CLVB_PrintRegisters(UART *UART_USB);

#endif
