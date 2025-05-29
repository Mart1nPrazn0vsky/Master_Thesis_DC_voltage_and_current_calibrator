#include "CCB.h"


#define G							71
#define H							72
#define I							73
#define REG_G_SIZE		4
#define REG_H_SIZE		4
#define REG_I_SIZE		8

#define LED_4R 				14
#define LED_4G 				13
#define LED_3R 				12
#define LED_3G 				11
#define LED_2R 				10
#define LED_2G 				9
#define LED_1R 				8
#define LED_1G 				7
#define DIT						4
#define K4						3
#define K3						2
#define K2						1
#define K1						0

#define LED_OFF				0
#define LED_ON				1

const double CCB_DAC_resolution = 1048576;
const double CCB_DAC_resolution_dith = 16777216;
const double CCB_R1_gain = 10.0;
const double CCB_R2_gain = 100.0;
const double CCB_R3_gain = 1000.0;
const double CCB_VREFPF = 5.0;
const double CCB_VREFNF = 0.0;
const double CCB_RREF = 2256.25;

const double CCB_R1_max = 0.022;
const double CCB_R1_min = 0.0;
const double CCB_R2_max = 0.22;
const double CCB_R2_min = 0.0;
const double CCB_R3_max = 2.2;
const double CCB_R3_min = 0.0;

volatile static uint8_t string[100];

volatile static uint16_t register_G = 0x0000;
volatile static uint16_t register_H = 0x0000;
volatile static uint32_t register_I = 0x00000000;

UART* UART_CCB;
CCB_module_state CCB_state;

uint8_t CCB_Init(UART *UART_handle)
{
	uint8_t error = NO_ERROR;
	UART_CCB = UART_handle;
	uint8_t string[10];
	
	delay_ms(100);
	
	for (uint8_t i = 0; i < 3; i++)				//try connect to CLVB module 3 times
	{
		error = Module_GetName(UART_handle, string);
		if (error == NO_ERROR) {break;}			//if no error occured, end this loop
		delay_ms(1000);											//delay between attempts
	}
	
	if (error == NO_ERROR)
	{
		if (strcmp(string, "@CCB") != 0)		//if strings are same, strcmp returns 0
		{
			error = ERROR_WRONG_MODULE;
			return error;
		}
	}
	else
	{
		return error;
	}
	
	error = CCB_SetRange(3);
	error = CCB_SetCurrent(0.0);
	
	//turn on/off LEDs
	
	CCB_state.current = 0.0;
	CCB_state.range = 3;
	CCB_state.output_state = CCB_OUTPUT_OFF;
	CCB_state.autorange_state = CCB_AUTORANGE_OFF;
	CCB_state.dithering_state = CCB_DITHERING_OFF;
	
	return error;
}


uint8_t CCB_TurnOFFModule(void)
{
	uint8_t error;
	
	error = CCB_SetCurrent(0.0);
	error = CCB_OutputOFF();
	
	return error;
}


uint8_t CCB_SetRange(uint8_t range)
{
	uint8_t error = NO_ERROR;
	
	if (range == 1)
	{
		register_H = Utils_ClearBit(register_H, K1);
		register_H = Utils_ClearBit(register_H, K2);
	}
	else if (range == 2)
	{
		register_H = Utils_SetBit(register_H, K1);
		register_H = Utils_ClearBit(register_H, K2);
	}
	else if (range == 3)
	{
		register_H = Utils_SetBit(register_H, K1);
		register_H = Utils_SetBit(register_H, K2);
	}
	
	error = Module_WriteToRegister(UART_CCB, H, register_H, REG_H_SIZE);
	if (error == NO_ERROR) {CCB_state.range = range;}
	
	return error;
}


uint8_t CCB_CheckRange(double current)
{
	uint8_t error = NO_ERROR;
	
	if ((CCB_state.range == 1) && ((current > CCB_R1_max) || (current < CCB_R1_min))) {error = ERROR_VOLT_RANGE;}
	else if ((CCB_state.range == 2) && ((current > CCB_R2_max) || (current < CCB_R2_min))) {error = ERROR_VOLT_RANGE;}
	else if ((CCB_state.range == 3) && ((current > CCB_R3_max) || (current < CCB_R3_min))) {error = ERROR_VOLT_RANGE;}
	
	return error;
}


uint8_t CCB_Autorange(double current)
{
	uint8_t error = NO_ERROR;
	
	if ((current <= CCB_R1_max) && (current >= CCB_R1_min)) {error = CCB_CheckRange(1);}
	else if ((current <= CCB_R2_max) && (current >= CCB_R2_min)) {error = CCB_CheckRange(2);}
	else if ((current <= CCB_R3_max) && (current >= CCB_R3_min)) {error = CCB_CheckRange(3);}
	else {error = ERROR_VOLT_RANGE;}
	
	return error;
}


void CCB_AutorangeON(void)
{
	CCB_state.autorange_state = CCB_AUTORANGE_ON;
}


void CCB_AutorangeOFF(void)
{
	CCB_state.autorange_state = CCB_AUTORANGE_OFF;
}


uint8_t CCB_OutputON(void)
{
	uint8_t error = NO_ERROR;
	
	register_H = Utils_SetBit(register_H, K4);
	error = Module_WriteToRegister(UART_CCB, H, register_H, REG_H_SIZE);
	if (error == NO_ERROR) {CCB_state.output_state = CCB_OUTPUT_ON;}
	
	return error;
}


uint8_t CCB_OutputOFF(void)
{
	uint8_t error = NO_ERROR;
	
	register_H = Utils_ClearBit(register_H, K4);
	error = Module_WriteToRegister(UART_CCB, H, register_H, REG_H_SIZE);
	if (error == NO_ERROR) {CCB_state.output_state = CCB_OUTPUT_OFF;}
	
	return error;
}


uint8_t CCB_DitheringON(void)
{
	uint8_t error = NO_ERROR;
	
	register_H = Utils_SetBit(register_H, DIT);
	error = Module_WriteToRegister(UART_CCB, H, register_H, REG_H_SIZE);
	if (error == NO_ERROR) {CCB_state.dithering_state = CCB_DITHERING_ON;}
	
	return error;
}


uint8_t CCB_DitheringOFF(void)
{
	uint8_t error = NO_ERROR;
	
	register_H = Utils_ClearBit(register_H, DIT);
	error = Module_WriteToRegister(UART_CCB, H, register_H, REG_H_SIZE);
	if (error == NO_ERROR) {CCB_state.dithering_state = CCB_DITHERING_OFF;}
	
	return error;
}


uint8_t CCB_SetCurrent(double current)
{
	uint8_t error = NO_ERROR;
	
	//handle range
	if (CCB_state.autorange_state == CCB_AUTORANGE_OFF)
	{
		error = CCB_CheckRange(current);				//if autorange is OFF, check if voltage is within selected range
	}
	else {error = CCB_Autorange(current);}		//if autorange is ON, find correct range and set it
	if (error != NO_ERROR) {return error;}		//in case of any problem, return error
	
	//handle voltage
	if (CCB_state.dithering_state == CCB_DITHERING_OFF)
	{
		register_I = (CCB_GetVoltageCode(current) << 4);											//update register I without dithering
	}
	else {register_I = CCB_GetVoltageCode(current);}												//update register I
	error = Module_WriteToRegister(UART_CCB, I, register_I, REG_I_SIZE);		//update register I
	if (error != NO_ERROR) {return error;}		//in case of any problem, return error
	else {CCB_state.current = current;}
	
	return error;
}


uint32_t CCB_GetVoltageCode(double current)
{
	uint32_t code = 0x00000000;
	
	if (CCB_state.dithering_state == 0)		//if dithering is off
	{
		if (CCB_state.range == 1)
		{
			code = (uint32_t) ((((current - CCB_R1_offset_error) * CCB_RREF) / (CCB_R1_gain * CCB_R1_gain_error) - CCB_VREFNF) * (CCB_DAC_resolution / (CCB_VREFPF - CCB_VREFNF)));
		}
		else if (CCB_state.range == 2)
		{
			code = (uint32_t) ((((current - CCB_R2_offset_error) * CCB_RREF) / (CCB_R2_gain * CCB_R2_gain_error) - CCB_VREFNF) * (CCB_DAC_resolution / (CCB_VREFPF - CCB_VREFNF)));
		}
		else if (CCB_state.range == 3)
		{
			code = (uint32_t) ((((current - CCB_R3_offset_error) * CCB_RREF) / (CCB_R3_gain * CCB_R3_gain_error) - CCB_VREFNF) * (CCB_DAC_resolution / (CCB_VREFPF - CCB_VREFNF)));
		}
	}
	else		//if dithering is on
	{
		if (CCB_state.range == 1)
		{
			code = (uint32_t) ((((current - CCB_R1_offset_error) * CCB_RREF) / (CCB_R1_gain * CCB_R1_gain_error) - CCB_VREFNF) * (CCB_DAC_resolution_dith / (CCB_VREFPF - CCB_VREFNF)));
		}
		else if (CCB_state.range == 2)
		{
			code = (uint32_t) ((((current - CCB_R2_offset_error) * CCB_RREF) / (CCB_R2_gain * CCB_R2_gain_error) - CCB_VREFNF) * (CCB_DAC_resolution_dith / (CCB_VREFPF - CCB_VREFNF)));
		}
		else if (CCB_state.range == 3)
		{
			code = (uint32_t) ((((current - CCB_R3_offset_error) * CCB_RREF) / (CCB_R3_gain * CCB_R3_gain_error) - CCB_VREFNF) * (CCB_DAC_resolution_dith / (CCB_VREFPF - CCB_VREFNF)));
		}
	}
	
	return code;
}


uint8_t CCB_SetLED(uint8_t led, uint8_t value)
{
	uint8_t error = NO_ERROR;
	
	if (value == LED_OFF) {register_H = Utils_ClearBit(register_H, led);}		//turn OFF LED
	else {register_H = Utils_SetBit(register_H, led);}											//turn ON LED
	error = Module_WriteToRegister(UART_CCB, H, register_H, REG_H_SIZE);
	
	return error;
}


double CCB_GetCurrent(void)
{
	return CCB_state.current;
}


uint8_t CCB_GetRange(void)
{
	return CCB_state.range;
}


uint8_t CCB_GetAutorangeState(void)
{
	return CCB_state.autorange_state;
}


uint8_t CCB_GetOutputState(void)
{
	return CCB_state.output_state;
}


uint8_t CCB_GetDitheringState(void)
{
	return CCB_state.dithering_state;
}


uint8_t CCB_PrintRegisters(UART *UART_USB)
{
	uint8_t error = NO_ERROR;
	uint32_t data;
	
	error = Module_ReadRegister(UART_CCB, G, &data);
	if (error != NO_ERROR) {return error;}
	register_G = data;
	error = Module_ReadRegister(UART_CCB, H, &data);
	if (error != NO_ERROR) {return error;}
	register_H = data;
	error = Module_ReadRegister(UART_CCB, I, &data);
	if (error != NO_ERROR) {return error;}
	register_I = data;
	
	uint8_t string_G[20];
	uint8_t string_H[20];
	uint8_t string_I[20];
	
	Utils_IntToHexString(register_G, string_G, 4);
	Utils_IntToHexString(register_H, string_H, 4);
	Utils_IntToHexString(register_I, string_I, 8);
	
	UART_SendString(UART_CCB, "CCB:\n");
	UART_SendString(UART_CCB, "G: ");
	UART_SendString(UART_CCB, string_G);
	UART_SendString(UART_CCB, "\n");
	UART_SendString(UART_CCB, "H: ");
	UART_SendString(UART_CCB, string_H);
	UART_SendString(UART_CCB, "\n");
	UART_SendString(UART_CCB, "I: ");
	UART_SendString(UART_CCB, string_I);
	UART_SendString(UART_CCB, "\n");
	
	return error;
}