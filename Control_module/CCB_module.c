#include "CCB_module.h"

#define REGISTER_G		71
#define REGISTER_H		72
#define REGISTER_I		73

#define LED_4R 		14
#define LED_4G 		13
#define LED_3R 		12
#define LED_3G 		11
#define LED_2R 		10
#define LED_2G 		9
#define LED_1R 		8
#define LED_1G 		7
#define DIT				4
#define K4				3
#define K3				2
#define K2				1
#define K1				0

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

volatile static uint16_t register_G_received = 0x0000;
volatile static uint16_t register_H_received = 0x0000;
volatile static uint32_t register_I_received = 0x00000000;

volatile static uint8_t CCB_dith_on = 0;
volatile static uint8_t CCB_range = 3;
volatile static uint8_t CCB_output_state = CCB_OUTPUT_OFF;


UART* UART_CCB;
CCB_module_state* CCB_state;


uint8_t CCB_Init(UART* UART_handle)
{
	uint8_t error = NO_ERROR;
	UART_CCB = UART_handle;
	
	delay_ms(100);
	
	for (uint8_t i = 0; i < 3; i++)				//try connect to CLVB module 3 times
	{
		error = CCB_ReadAllRegisters();			//try to read name of module and content of all registers
		if (error == NO_ERROR) {break;}			//if received data are OK, break this loop and continue
		delay_ms(1000);											//delay between attempts
	}
	
	if (error != NO_ERROR) {return error;}
	
	error = CCB_SetRange(1);
	error = CCB_SetCurrent(0.0);
	
	//turn on/off LEDs
	
	CCB_state->current = 0.0;
	CCB_state->range = 1;
	CCB_state->output_state = CCB_OUTPUT_OFF;
	CCB_state->autorange_state = CCB_AUTORANGE_OFF;
	CCB_state->dithering_state = CCB_DITHERING_OFF;
	
	return error;
}


uint8_t CCB_WriteToRegister(uint8_t reg, uint32_t data)
{
	uint8_t error = NO_ERROR;
	
	if ((reg == REGISTER_G) || (reg == REGISTER_H)) {Utils_IntToHexString(data, string, 4);}		//registers G, H
	else {Utils_IntToHexString(data, string, 8);}																								//registers I, J
	UART_SendByte(UART_CCB, reg);								//send first letter (name of register)
	UART_SendString(UART_CCB, string);					//send hexadecimal number in string format
	UART_SendByte(UART_CCB, '\n');							//send end of message
	UART_SendByte(UART_CCB, '\r');							//send end of message
	
	delay_ms(10);
	
	error = CCB_ReadAllRegisters();							//read registers from CLVB
	if (error == NO_ERROR)
	{
		error = CCB_CompareRegisters();						//check if content of received registers is same as transmitted
	}
	
	return error;
}


uint8_t CCB_ReadAllRegisters(void)
{
	uint8_t error = ERROR_COMMUNICATION;
	UART_ClearRXBuffer(UART_CCB);		//clear RX buffer
	
	UART_SendString(UART_CCB, "G003F\n");
	delay_ms(100);
	if (UART_AvailableBytes(UART_CCB) > 0) {UART_ReadLine(UART_CCB, string);}		//if data were received, read one line
	else {return error;}																													//if no data were received, return error
	
	if (strcmp(string, "@CCB\n\r"))
	{
		while (UART_AvailableBytes(UART_CCB) > 0)
		{
			UART_ReadLine(UART_CCB, string);
			if (string[0] == 'G') {register_G_received = Utils_HexStringToInt(string, 4);}
			else if (string[0] == 'H') {register_H_received = Utils_HexStringToInt(string, 4);}
			else if (string[0] == 'I') {register_I_received = Utils_HexStringToInt(string, 8);}
		}
		error = NO_ERROR;											//clear error flag
	}	
	
	UART_ClearRXBuffer(UART_CCB);		//clear RX buffer
	
	return error;
}


uint8_t CCB_CompareRegisters(void)
{
	uint8_t error = 0;
	
	if (register_G != register_G_received) {error = 1;}
	if (register_H != register_H_received) {error = 1;}
	if (register_I != register_I_received) {error = 1;}
	
	return error;
}


uint8_t CCB_SetRange(uint8_t range)
{
	uint8_t error = NO_ERROR;
	
	if (range == 1)
	{
		Utils_ClearBit(register_H, K1);
		Utils_ClearBit(register_H, K2);
	}
	else if (range == 2)
	{
		Utils_SetBit(register_H, K1);
		Utils_ClearBit(register_H, K2);
	}
	else if (range == 3)
	{
		Utils_SetBit(register_H, K1);
		Utils_SetBit(register_H, K2);
	}
	
	error = CCB_WriteToRegister(REGISTER_H, register_H);
	if (error == NO_ERROR) {CCB_state->range = range;}
	
	return error;
}


void CCB_AutorangeON(void)
{
	CCB_state->autorange_state = CCB_AUTORANGE_ON;
}


void CCB_AutorangeOFF(void)
{
	CCB_state->autorange_state = CCB_AUTORANGE_OFF;
}


uint8_t CCB_OutputON(void)
{
	uint8_t error = NO_ERROR;
	Utils_SetBit(register_H, K4);
	error = CCB_WriteToRegister(REGISTER_H, register_H);
	if (error == NO_ERROR) {CCB_state->output_state = CCB_OUTPUT_ON;}
	return error;
}

uint8_t CCB_OutputOFF(void)
{
	uint8_t error = NO_ERROR;
	Utils_ClearBit(register_H, K4);
	error = CCB_WriteToRegister(REGISTER_H, register_H);
	if (error == NO_ERROR) {CCB_state->output_state = CCB_OUTPUT_OFF;}
	return error;
}


uint8_t CCB_DitheringON(void)
{
	uint8_t error = NO_ERROR;
	Utils_SetBit(register_H, DIT);
	error = CCB_WriteToRegister(REGISTER_H, register_H);
	if (error == NO_ERROR) {CCB_state->dithering_state = CCB_DITHERING_ON;}
	return error;
}


uint8_t CCB_DitheringOFF(void)
{
	uint8_t error = NO_ERROR;
	Utils_ClearBit(register_H, DIT);
	error = CCB_WriteToRegister(REGISTER_H, register_H);
	if (error == NO_ERROR) {CCB_state->dithering_state = CCB_DITHERING_OFF;}
	return error;
}


uint8_t CCB_CheckRange(double current)
{
	uint8_t error = NO_ERROR;
	
	if ((CCB_state->range == 1) && ((current > CCB_R1_max) || (current < CCB_R1_min))) {error = ERROR_VOLT_RANGE;}
	else if ((CCB_state->range == 2) && ((current > CCB_R2_max) || (current < CCB_R2_min))) {error = ERROR_VOLT_RANGE;}
	else if ((CCB_state->range == 3) && ((current > CCB_R3_max) || (current < CCB_R3_min))) {error = ERROR_VOLT_RANGE;}
	
	return error;
}


uint8_t CCB_Autorange(double current)
{
	uint8_t error = NO_ERROR;
	if ((current < CCB_R1_max) && (current > CCB_R1_min)) {error = CCB_SetRange(1);}
	else if ((current < CCB_R2_max) && (current > CCB_R2_min)) {error = CCB_SetRange(2);}
	else if ((current < CCB_R3_max) && (current > CCB_R3_min)) {error = CCB_SetRange(3);}
	else {error = ERROR_VOLT_RANGE;}
	return error;
}


uint8_t CCB_SetCurrent(double current)
{
	uint8_t error = NO_ERROR;
	
	//handle range
	if (CCB_state->autorange_state == 0) {error = CCB_CheckRange(current);}		//if autorange is OFF, check if voltage is within selected range
	else {error = CCB_Autorange(current);}																		//if autorange is ON, find correct range and set it
	if (error != NO_ERROR) {return error;}																		//in case of any problem, return error
	
	//handle current
	if (CCB_state->dithering_state == CCB_DITHERING_OFF)
	{
		register_I = (CCB_GetVoltageCode(current) << 4);			//update register I without dithering
	}
	else {register_I = CCB_GetVoltageCode(current);}				//update register I
	error = CCB_WriteToRegister(REGISTER_I, register_I);			
	if (error != NO_ERROR) {return error;}
	else {CCB_state->current = current;}
	
	return error;
}


uint32_t CCB_GetVoltageCode(double current)
{
	uint32_t code = 0x00000000;
	
	if (CCB_state->dithering_state == 0)		//if dithering is off
	{
		if (CCB_range == 1)
		{
			code = (uint32_t) ((((current - CCB_R1_offset_error) * CCB_RREF) / (CCB_R1_gain * CCB_R1_gain_error) - CCB_VREFNF) * (CCB_DAC_resolution / (CCB_VREFPF - CCB_VREFNF)));
		}
		else if (CCB_range == 2)
		{
			code = (uint32_t) ((((current - CCB_R2_offset_error) * CCB_RREF) / (CCB_R2_gain * CCB_R2_gain_error) - CCB_VREFNF) * (CCB_DAC_resolution / (CCB_VREFPF - CCB_VREFNF)));
		}
		else if (CCB_range == 3)
		{
			code = (uint32_t) ((((current - CCB_R3_offset_error) * CCB_RREF) / (CCB_R3_gain * CCB_R3_gain_error) - CCB_VREFNF) * (CCB_DAC_resolution / (CCB_VREFPF - CCB_VREFNF)));
		}
	}
	else		//if dithering is on
	{
		if (CCB_range == 1)
		{
			code = (uint32_t) ((((current - CCB_R1_offset_error) * CCB_RREF) / (CCB_R1_gain * CCB_R1_gain_error) - CCB_VREFNF) * (CCB_DAC_resolution_dith / (CCB_VREFPF - CCB_VREFNF)));
		}
		else if (CCB_range == 2)
		{
			code = (uint32_t) ((((current - CCB_R2_offset_error) * CCB_RREF) / (CCB_R2_gain * CCB_R2_gain_error) - CCB_VREFNF) * (CCB_DAC_resolution_dith / (CCB_VREFPF - CCB_VREFNF)));
		}
		else if (CCB_range == 3)
		{
			code = (uint32_t) ((((current - CCB_R3_offset_error) * CCB_RREF) / (CCB_R3_gain * CCB_R3_gain_error) - CCB_VREFNF) * (CCB_DAC_resolution_dith / (CCB_VREFPF - CCB_VREFNF)));
		}
	}
	
	return code;
}


uint8_t CCB_TurnOFFModule(void)
{
	uint8_t error;
	
	error = CCB_SetCurrent(0.0);
	error = CCB_OutputOFF();
	
	return error;
}


uint8_t CCB_GetRange(void)
{
	return CCB_range;
}


uint8_t CCB_GetOutputState(void)
{
	return CCB_output_state;
}


void CCB_GetModuleState(CCB_module_state *handle)
{
	handle->current = CCB_state->current;
	handle->range = CCB_state->range;
	handle->output_state = CCB_state->output_state;
	handle->autorange_state = CCB_state->autorange_state;
	handle->dithering_state = CCB_state->dithering_state;
}
