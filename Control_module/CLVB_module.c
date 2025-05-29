#include "CLVB_module.h"


#define REGISTER_G		71
#define REGISTER_H		72
#define REGISTER_I		73
#define REGISTER_J		74

#define LED_4R 				14
#define LED_4G 				13
#define LED_3R 				12
#define LED_3G 				11
#define LED_2R 				10
#define LED_2G 				9
#define LED_1R 				8
#define LED_1G 				7
#define AC						4
#define DIT						3
#define K3						2
#define K2						1
#define K1						0

const double CLVB_DAC_resolution = 1048576;
const double CLVB_DAC_resolution_dith = 16777216;
const double CLVB_R1_gain = 1.125 * 2.0 / 100.0;
const double CLVB_R2_gain = 1.125 * 2.0 / 10.0;
const double CLVB_R3_gain = 1.125 * 2.0 / 1.0;
const double CLVB_VREFPF = 10.0;
const double CLVB_VREFNF = -10.0;
const double CLVB_AC_sampling_freq = 100000.0;
const double CLVB_freq_resolution = 4294967296;

const double CLVB_R1_max = 0.22;
const double CLVB_R1_min = -0.22;
const double CLVB_R2_max = 2.2;
const double CLVB_R2_min = -2.2;
const double CLVB_R3_max = 22.0;
const double CLVB_R3_min = -22.0;

const double CLVB_freq_max = 10000.0;
const double CLVB_freq_min = 0.0;

volatile static uint8_t string[100];

/*volatile static uint16_t register_G = 0x0000;
volatile static uint16_t register_H = 0x0000;
volatile static uint32_t register_I = 0x00000000;
volatile static uint32_t register_J = 0x00000000;

volatile static uint16_t register_G_received = 0x0000;
volatile static uint16_t register_H_received = 0x0000;
volatile static uint32_t register_I_received = 0x00000000;
volatile static uint32_t register_J_received = 0x00000000;*/


UART* UART_CLVB;
CLVB_module_state* CLVB_state;
module *CLVB_module;



CLVB_module_state *CLVB_CreateModule(void)
{
	return CLVB_state;
}


uint8_t CLVB_Init(UART* UART_handle)
{
	uint8_t error = NO_ERROR;
	UART_CLVB = UART_handle;
	
	delay_ms(100);
	
	for (uint8_t i = 0; i < 3; i++)				//try connect to CLVB module 3 times
	{
		error = Module_ReadAllRegisters(CLVB_module, UART_CLVB);		//try to read name of module and content of all registers
		if (error == NO_ERROR) {break;}			//if received data are OK, break this loop and continue
		delay_ms(1000);											//delay between attempts
	}
	
	if (error != NO_ERROR) {return error;}
	
	error = CLVB_SetRange(3);
	error = CLVB_SetVoltageDC(0.0);
	
	//turn on/off LEDs
	
	CLVB_state->voltage = 0.0;
	CLVB_state->frequency = 0.0;
	CLVB_state->range = 3;
	CLVB_state->mode = CLVB_MODE_DC;
	CLVB_state->output_state = CLVB_OUTPUT_OFF;
	CLVB_state->autorange_state = CLVB_AUTORANGE_OFF;
	CLVB_state->dithering_state = CLVB_DITHERING_OFF;
	
	return error;
}


uint8_t CLVB_SetRange(uint8_t range)
{
	uint8_t error = NO_ERROR;
	
	if (range == 1)
	{
		Utils_SetBit(CLVB_module->reg_H, K2);
		Utils_SetBit(CLVB_module->reg_H, K3);
	}
	else if (range == 2)
	{
		Utils_ClearBit(CLVB_module->reg_H, K2);
		Utils_SetBit(CLVB_module->reg_H, K3);
	}
	else if (range == 3)
	{
		Utils_ClearBit(CLVB_module->reg_H, K2);
		Utils_ClearBit(CLVB_module->reg_H, K3);
	}
	
	error = Module_UpdateRegister(CLVB_module, UART_CLVB, REGISTER_H);
	if (error == NO_ERROR) {CLVB_state->range = range;}
	
	return error;
}


void CLVB_AutorangeON(void)
{
	CLVB_state->autorange_state = CLVB_AUTORANGE_ON;
}


void CLVB_AutorangeOFF(void)
{
	CLVB_state->autorange_state = CLVB_AUTORANGE_OFF;
}


uint8_t CLVB_OutputON(void)
{
	uint8_t error = NO_ERROR;
	*(CLVB_module->reg_H) = Utils_SetBit(CLVB_module->reg_H, K1);
	error = Module_UpdateRegister(CLVB_module, UART_CLVB, REGISTER_H);
	if (error == NO_ERROR) {CLVB_state->output_state = CLVB_OUTPUT_ON;}
	return error;
}

uint8_t CLVB_OutputOFF(void)
{
	uint8_t error = NO_ERROR;
	Utils_ClearBit(CLVB_module->reg_H, K1);
	error = Module_UpdateRegister(CLVB_module, UART_CLVB, REGISTER_H);
	if (error == NO_ERROR) {CLVB_state->output_state = CLVB_OUTPUT_OFF;}
	return error;
}

uint8_t CLVB_DitheringON(void)
{
	uint8_t error = NO_ERROR;
	Utils_SetBit(CLVB_module->reg_H, DIT);
	error = Module_UpdateRegister(CLVB_module, UART_CLVB, REGISTER_H);
	if (error == NO_ERROR) {CLVB_state->dithering_state = CLVB_DITHERING_ON;}
	return error;
}


uint8_t CLVB_DitheringOFF(void)
{
	uint8_t error = NO_ERROR;
	Utils_ClearBit(CLVB_module->reg_H, DIT);
	error = Module_UpdateRegister(CLVB_module, UART_CLVB, REGISTER_H);
	if (error == NO_ERROR) {CLVB_state->dithering_state = CLVB_DITHERING_OFF;}
	return error;
}


uint8_t CLVB_CheckRange(double voltage)
{
	uint8_t error = NO_ERROR;
	
	if ((CLVB_state->range == 1) && ((voltage > CLVB_R1_max) || (voltage < CLVB_R1_min))) {error = ERROR_VOLT_RANGE;}
	else if ((CLVB_state->range == 2) && ((voltage > CLVB_R2_max) || (voltage < CLVB_R2_min))) {error = ERROR_VOLT_RANGE;}
	else if ((CLVB_state->range == 3) && ((voltage > CLVB_R3_max) || (voltage < CLVB_R3_min))) {error = ERROR_VOLT_RANGE;}
	
	return error;
}


uint8_t CLVB_Autorange(double voltage)
{
	uint8_t error = NO_ERROR;
	if ((voltage < CLVB_R1_max) && (voltage > CLVB_R1_min)) {error = CLVB_SetRange(1);}
	else if ((voltage < CLVB_R2_max) && (voltage > CLVB_R2_min)) {error = CLVB_SetRange(2);}
	else if ((voltage < CLVB_R3_max) && (voltage > CLVB_R3_min)) {error = CLVB_SetRange(3);}
	else {error = ERROR_VOLT_RANGE;}
	return error;
}


uint8_t CLVB_SetVoltageDC(double voltage)
{
	uint8_t error = NO_ERROR;
	
	//handle range
	if (CLVB_state->autorange_state == 0) {error = CLVB_CheckRange(voltage);}		//if autorange is OFF, check if voltage is within selected range
	else {error = CLVB_Autorange(voltage);}																			//if autorange is ON, find correct range and set it
	if (error != NO_ERROR) {return error;}																			//in case of any problem, return error
	
	//handle mode
	Utils_ClearBit(CLVB_module->reg_H, AC);														//turn off AC mode
	error = Module_UpdateRegister(CLVB_module, UART_CLVB, REGISTER_H);
	if (error != NO_ERROR) {return error;}
	else {CLVB_state->mode = CLVB_MODE_DC;}
	
	//handle voltage
	if (CLVB_state->dithering_state == CLVB_DITHERING_OFF)
	{
		CLVB_module->reg_I = (CLVB_GetVoltageCode(voltage) << 4);			//update register I without dithering
	}
	else {CLVB_module->reg_I = CLVB_GetVoltageCode(voltage);}				//update register I
	error = Module_UpdateRegister(CLVB_module, UART_CLVB, REGISTER_H);
	if (error != NO_ERROR) {return error;}
	else {CLVB_state->voltage = voltage;}
	
	return error;
}


uint8_t CLVB_SetVoltageAC(double voltage, double frequency)
{
	uint8_t error = NO_ERROR;
	
	//handle range
	if (CLVB_state->autorange_state == 0) {error = CLVB_CheckRange(voltage);}		//if autorange is OFF, check if voltage is within selected range
	else {error = CLVB_Autorange(voltage);}																			//if autorange is ON, find correct range and set it
	if (error != NO_ERROR) {return error;}																			//in case of any problem, return error
	
	//handle frequency
	error = CLVB_SetFrequency(frequency);			//set frequency
	if (error != NO_ERROR) {return error;}
	
	//handle voltage
	CLVB_module->reg_I = (CLVB_GetVoltageCode(voltage) << 4);					//calculate corect voltage code for DAC
	error = Module_UpdateRegister(CLVB_module, UART_CLVB, REGISTER_I);
	if (error != NO_ERROR) {return error;}
	else {CLVB_state->voltage = voltage;}
	
	//handle mode
	Utils_SetBit(CLVB_module->reg_H, AC);															//turn on AC mode
	error = Module_UpdateRegister(CLVB_module, UART_CLVB, REGISTER_H);
	if (error != NO_ERROR) {return error;}
	else {CLVB_state->mode = CLVB_MODE_AC;}
	
	return error;
}


uint32_t CLVB_GetVoltageCode(double voltage)
{
	uint32_t code = 0x00000000;
	
	if (CLVB_state->dithering_state == 0)		//if dithering is off
	{
		if (CLVB_state->range == 1)
		{
			code = (uint32_t) (((voltage - CLVB_R1_offset_error) / (CLVB_R1_gain * CLVB_R1_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution / (CLVB_VREFPF - CLVB_VREFNF)));
		}
		else if (CLVB_state->range == 2)
		{
			code = (uint32_t) (((voltage - CLVB_R2_offset_error) / (CLVB_R2_gain * CLVB_R2_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution / (CLVB_VREFPF - CLVB_VREFNF)));
		}
		else if (CLVB_state->range == 3)
		{
			code = (uint32_t) (((voltage - CLVB_R3_offset_error) / (CLVB_R3_gain * CLVB_R3_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution / (CLVB_VREFPF - CLVB_VREFNF)));
		}
	}
	else		//if dithering is on
	{
		if (CLVB_state->range == 1)
		{
			code = (uint32_t) (((voltage - CLVB_R1_offset_error) / (CLVB_R1_gain * CLVB_R1_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution_dith / (CLVB_VREFPF - CLVB_VREFNF)));
		}
		else if (CLVB_state->range == 2)
		{
			code = (uint32_t) (((voltage - CLVB_R2_offset_error) / (CLVB_R2_gain * CLVB_R2_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution_dith / (CLVB_VREFPF - CLVB_VREFNF)));
		}
		else if (CLVB_state->range == 3)
		{
			code = (uint32_t) (((voltage - CLVB_R3_offset_error) / (CLVB_R3_gain * CLVB_R3_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution_dith / (CLVB_VREFPF - CLVB_VREFNF)));
		}
	}
	
	return code;
}


uint8_t CLVB_CheckFrequency(double frequency)
{
	uint8_t error = 0;
	
	if ((frequency > CLVB_freq_max) || (frequency < CLVB_freq_min)) {error = ERROR_FREQ_RANGE;}
	
	return error;
}


uint8_t CLVB_SetFrequency(double frequency)
{
	//calculate frequency tuning word (FTW) and save it into register J
	//FTW is "step" which is added to phase accumulator at every sample
	
	uint8_t error = NO_ERROR;
	
	error = CLVB_CheckFrequency(frequency);
	if (error != NO_ERROR) {return error;}
	
	CLVB_module->reg_J = (uint32_t) ((frequency * CLVB_freq_resolution) / (CLVB_AC_sampling_freq * CLVB_FPGA_CLK_freq_correction_const));		//calculate FTW
	error = Module_UpdateRegister(CLVB_module, UART_CLVB, REGISTER_J);
	if (error != NO_ERROR) {return error;}
	else {CLVB_state->frequency = frequency;}
	
	return error;
}


uint8_t CLVB_TurnOFFModule(void)
{
	uint8_t error;
	
	error = CLVB_SetVoltageDC(0.0);
	error = CLVB_OutputOFF();
	
	return error;
}


double CLVB_GetVoltage(void)
{
	return CLVB_state->voltage;
}


double CLVB_GetFrequency(void)
{
	return CLVB_state->frequency;
}


uint8_t CLVB_GetRange(void)
{
	return CLVB_state->range;
}

uint8_t CLVB_GetMode(void)
{
	return CLVB_state->mode;
}


uint8_t CLVB_GetOutputState(void)
{
	return CLVB_state->output_state;
}


uint8_t CLVB_GetAutorangeState(void)
{
	return CLVB_state->autorange_state;
}


void CLVB_GetModuleState(CLVB_module_state *handle)
{
	handle->voltage = CLVB_state->voltage;
	handle->frequency = CLVB_state->frequency;
	handle->range = CLVB_state->range;
	handle->mode = CLVB_state->mode;
	handle->output_state = CLVB_state->output_state;
	handle->autorange_state = CLVB_state->autorange_state;
	handle->dithering_state = CLVB_state->dithering_state;
}



























/*CLVB_module_state *CLVB_CreateModule(void)
{
	
	return CLVB_state;
}


uint8_t CLVB_Init(UART* UART_handle)
{
	uint8_t error = NO_ERROR;
	UART_CLVB = UART_handle;
	
	delay_ms(100);
	
	for (uint8_t i = 0; i < 3; i++)				//try connect to CLVB module 3 times
	{
		error = CLVB_ReadAllRegisters();		//try to read name of module and content of all registers
		if (error == NO_ERROR) {break;}			//if received data are OK, break this loop and continue
		delay_ms(1000);											//delay between attempts
	}
	
	if (error != NO_ERROR) {return error;}
	
	error = CLVB_SetRange(3);
	error = CLVB_SetVoltageDC(0.0);
	
	//turn on/off LEDs
	
	CLVB_state->voltage = 0.0;
	CLVB_state->frequency = 0.0;
	CLVB_state->range = 3;
	CLVB_state->mode = CLVB_MODE_DC;
	CLVB_state->output_state = CLVB_OUTPUT_OFF;
	CLVB_state->autorange_state = CLVB_AUTORANGE_OFF;
	CLVB_state->dithering_state = CLVB_DITHERING_OFF;
	
	return error;
}


uint8_t CLVB_WriteToRegister(uint8_t reg, uint32_t data)
{
	uint8_t error = NO_ERROR;
	
	if ((reg == REGISTER_G) || (reg == REGISTER_H)) {Utils_IntToHexString(data, string, 4);}		//registers G, H
	else {Utils_IntToHexString(data, string, 8);}																								//registers I, J
	UART_SendByte(UART_CLVB, reg);							//send first letter (name of register)
	UART_SendString(UART_CLVB, string);					//send hexadecimal number in string format
	UART_SendByte(UART_CLVB, '\n');							//send end of message
	UART_SendByte(UART_CLVB, '\r');							//send end of message
	
	delay_ms(10);
	
	error = CLVB_ReadAllRegisters();						//read registers from CLVB
	if (error == NO_ERROR)
	{
		error = CLVB_CompareRegisters();					//check if content of received registers is same as transmitted
	}
	
	return error;
}


uint8_t CLVB_ReadAllRegisters(void)
{
	uint8_t error = ERROR_COMMUNICATION;
	UART_ClearRXBuffer(UART_CLVB);		//clear RX buffer
	
	UART_SendString(UART_CLVB, "G003F\n");
	delay_ms(100);
	if (UART_AvailableBytes(UART_CLVB) > 0) {UART_ReadLine(UART_CLVB, string);}		//if data were received, read one line
	else {return error;}																													//if no data were received, return error
	
	if (strcmp(string, "@CLVB\n\r"))
	{
		while (UART_AvailableBytes(UART_CLVB) > 0)
		{
			UART_ReadLine(UART_CLVB, string);
			if (string[0] == 'G') {register_G_received = Utils_HexStringToInt(string, 4);}
			else if (string[0] == 'H') {register_H_received = Utils_HexStringToInt(string, 4);}
			else if (string[0] == 'I') {register_I_received = Utils_HexStringToInt(string, 8);}
			else if (string[0] == 'J') {register_J_received = Utils_HexStringToInt(string, 8);}
		}
		error = NO_ERROR;								//clear error flag
	}	
	
	UART_ClearRXBuffer(UART_CLVB);		//clear RX buffer
	
	return error;
}


uint8_t CLVB_CompareRegisters(void)
{
	uint8_t error = NO_ERROR;
	
	if (register_G != register_G_received) {error = ERROR_COMMUNICATION;}
	if (register_H != register_H_received) {error = ERROR_COMMUNICATION;}
	if (register_I != register_I_received) {error = ERROR_COMMUNICATION;}
	if (register_J != register_J_received) {error = ERROR_COMMUNICATION;}
	
	return error;
}


uint8_t CLVB_SetRange(uint8_t range)
{
	uint8_t error = NO_ERROR;
	
	if (range == 1)
	{
		Utils_SetBit(register_H, K2);
		Utils_SetBit(register_H, K3);
	}
	else if (range == 2)
	{
		Utils_ClearBit(register_H, K2);
		Utils_SetBit(register_H, K3);
	}
	else if (range == 3)
	{
		Utils_ClearBit(register_H, K2);
		Utils_ClearBit(register_H, K3);
	}
	
	error = CLVB_WriteToRegister(REGISTER_H, register_H);
	if (error == NO_ERROR) {CLVB_state->range = range;}
	
	return error;
}


void CLVB_AutorangeON(void)
{
	CLVB_state->autorange_state = CLVB_AUTORANGE_ON;
}


void CLVB_AutorangeOFF(void)
{
	CLVB_state->autorange_state = CLVB_AUTORANGE_OFF;
}


uint8_t CLVB_OutputON(void)
{
	uint8_t error = NO_ERROR;
	Utils_SetBit(register_H, K1);
	error = CLVB_WriteToRegister(REGISTER_H, register_H);
	if (error == NO_ERROR) {CLVB_state->output_state = CLVB_OUTPUT_ON;}
	return error;
}

uint8_t CLVB_OutputOFF(void)
{
	uint8_t error = NO_ERROR;
	Utils_ClearBit(register_H, K1);
	error = CLVB_WriteToRegister(REGISTER_H, register_H);
	if (error == NO_ERROR) {CLVB_state->output_state = CLVB_OUTPUT_OFF;}
	return error;
}

uint8_t CLVB_DitheringON(void)
{
	uint8_t error = NO_ERROR;
	Utils_SetBit(register_H, DIT);
	error = CLVB_WriteToRegister(REGISTER_H, register_H);
	if (error == NO_ERROR) {CLVB_state->dithering_state = CLVB_DITHERING_ON;}
	return error;
}


uint8_t CLVB_DitheringOFF(void)
{
	uint8_t error = NO_ERROR;
	Utils_ClearBit(REGISTER_H, DIT);
	error = CLVB_WriteToRegister(REGISTER_H, register_H);
	if (error == NO_ERROR) {CLVB_state->dithering_state = CLVB_DITHERING_OFF;}
	return error;
}


uint8_t CLVB_CheckRange(double voltage)
{
	uint8_t error = NO_ERROR;
	
	if ((CLVB_state->range == 1) && ((voltage > CLVB_R1_max) || (voltage < CLVB_R1_min))) {error = ERROR_VOLT_RANGE;}
	else if ((CLVB_state->range == 2) && ((voltage > CLVB_R2_max) || (voltage < CLVB_R2_min))) {error = ERROR_VOLT_RANGE;}
	else if ((CLVB_state->range == 3) && ((voltage > CLVB_R3_max) || (voltage < CLVB_R3_min))) {error = ERROR_VOLT_RANGE;}
	
	return error;
}


uint8_t CLVB_Autorange(double voltage)
{
	uint8_t error = NO_ERROR;
	if ((voltage < CLVB_R1_max) && (voltage > CLVB_R1_min)) {error = CLVB_SetRange(1);}
	else if ((voltage < CLVB_R2_max) && (voltage > CLVB_R2_min)) {error = CLVB_SetRange(2);}
	else if ((voltage < CLVB_R3_max) && (voltage > CLVB_R3_min)) {error = CLVB_SetRange(3);}
	else {error = ERROR_VOLT_RANGE;}
	return error;
}


uint8_t CLVB_SetVoltageDC(double voltage)
{
	uint8_t error = NO_ERROR;
	
	//handle range
	if (CLVB_state->autorange_state == 0) {error = CLVB_CheckRange(voltage);}		//if autorange is OFF, check if voltage is within selected range
	else {error = CLVB_Autorange(voltage);}																			//if autorange is ON, find correct range and set it
	if (error != NO_ERROR) {return error;}																			//in case of any problem, return error
	
	//handle mode
	Utils_ClearBit(register_H, AC);														//turn off AC mode
	error = CLVB_WriteToRegister(REGISTER_H, register_H);			//update register H
	if (error != NO_ERROR) {return error;}
	else {CLVB_state->mode = CLVB_MODE_DC;}
	
	//handle voltage
	if (CLVB_state->dithering_state == CLVB_DITHERING_OFF)
	{
		register_I = (CLVB_GetVoltageCode(voltage) << 4);			//update register I without dithering
	}
	else {register_I = CLVB_GetVoltageCode(voltage);}				//update register I
	error = CLVB_WriteToRegister(REGISTER_I, register_I);			
	if (error != NO_ERROR) {return error;}
	else {CLVB_state->voltage = voltage;}
	
	return error;
}


uint8_t CLVB_SetVoltageAC(double voltage, double frequency)
{
	uint8_t error = NO_ERROR;
	
	//handle range
	if (CLVB_state->autorange_state == 0) {error = CLVB_CheckRange(voltage);}		//if autorange is OFF, check if voltage is within selected range
	else {error = CLVB_Autorange(voltage);}																			//if autorange is ON, find correct range and set it
	if (error != NO_ERROR) {return error;}																			//in case of any problem, return error
	
	//handle frequency
	error = CLVB_SetFrequency(frequency);			//set frequency
	if (error != NO_ERROR) {return error;}
	
	//handle mode
	register_I = (CLVB_GetVoltageCode(voltage) << 4);					//calculate corect voltage code for DAC
	error = CLVB_WriteToRegister(REGISTER_I, register_I);			//update register I
	if (error != NO_ERROR) {return error;}
	else {CLVB_state->voltage = voltage;}
	
	//handle voltage
	Utils_SetBit(register_H, AC);															//turn on AC mode
	error = CLVB_WriteToRegister(REGISTER_H, register_H);			//update register H
	if (error != NO_ERROR) {return error;}
	else {CLVB_state->mode = CLVB_MODE_AC;}
	
	return error;
}


uint32_t CLVB_GetVoltageCode(double voltage)
{
	uint32_t code = 0x00000000;
	
	if (CLVB_state->dithering_state == 0)		//if dithering is off
	{
		if (CLVB_state->range == 1)
		{
			code = (uint32_t) (((voltage - CLVB_R1_offset_error) / (CLVB_R1_gain * CLVB_R1_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution / (CLVB_VREFPF - CLVB_VREFNF)));
		}
		else if (CLVB_state->range == 2)
		{
			code = (uint32_t) (((voltage - CLVB_R2_offset_error) / (CLVB_R2_gain * CLVB_R2_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution / (CLVB_VREFPF - CLVB_VREFNF)));
		}
		else if (CLVB_state->range == 3)
		{
			code = (uint32_t) (((voltage - CLVB_R3_offset_error) / (CLVB_R3_gain * CLVB_R3_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution / (CLVB_VREFPF - CLVB_VREFNF)));
		}
	}
	else		//if dithering is on
	{
		if (CLVB_state->range == 1)
		{
			code = (uint32_t) (((voltage - CLVB_R1_offset_error) / (CLVB_R1_gain * CLVB_R1_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution_dith / (CLVB_VREFPF - CLVB_VREFNF)));
		}
		else if (CLVB_state->range == 2)
		{
			code = (uint32_t) (((voltage - CLVB_R2_offset_error) / (CLVB_R2_gain * CLVB_R2_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution_dith / (CLVB_VREFPF - CLVB_VREFNF)));
		}
		else if (CLVB_state->range == 3)
		{
			code = (uint32_t) (((voltage - CLVB_R3_offset_error) / (CLVB_R3_gain * CLVB_R3_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution_dith / (CLVB_VREFPF - CLVB_VREFNF)));
		}
	}
	
	return code;
}


uint8_t CLVB_CheckFrequency(double frequency)
{
	uint8_t error = 0;
	
	if ((frequency > CLVB_freq_max) || (frequency < CLVB_freq_min)) {error = ERROR_FREQ_RANGE;}
	
	return error;
}


uint8_t CLVB_SetFrequency(double frequency)
{
	//calculate frequency tuning word (FTW) and save it into register J
	//FTW is "step" which is added to phase accumulator at every sample
	
	uint8_t error = NO_ERROR;
	
	error = CLVB_CheckFrequency(frequency);
	if (error != NO_ERROR) {return error;}
	
	register_J = (uint32_t) ((frequency * CLVB_freq_resolution) / (CLVB_AC_sampling_freq * CLVB_FPGA_CLK_freq_correction_const));		//calculate FTW
	error = CLVB_WriteToRegister(REGISTER_J, register_J);			//update register J
	if (error != NO_ERROR) {return error;}
	else {CLVB_state->frequency = frequency;}
	
	return error;
}


uint8_t CLVB_TurnOFFModule(void)
{
	uint8_t error;
	
	error = CLVB_SetVoltageDC(0.0);
	error = CLVB_OutputOFF();
	
	return error;
}


double CLVB_GetVoltage(void)
{
	return CLVB_state->voltage;
}


double CLVB_GetFrequency(void)
{
	return CLVB_state->frequency;
}


uint8_t CLVB_GetRange(void)
{
	return CLVB_state->range;
}

uint8_t CLVB_GetMode(void)
{
	return CLVB_state->mode;
}


uint8_t CLVB_GetOutputState(void)
{
	return CLVB_state->output_state;
}


uint8_t CLVB_GetAutorangeState(void)
{
	return CLVB_state->autorange_state;
}


void CLVB_GetModuleState(CLVB_module_state *handle)
{
	handle->voltage = CLVB_state->voltage;
	handle->frequency = CLVB_state->frequency;
	handle->range = CLVB_state->range;
	handle->mode = CLVB_state->mode;
	handle->output_state = CLVB_state->output_state;
	handle->autorange_state = CLVB_state->autorange_state;
	handle->dithering_state = CLVB_state->dithering_state;
}*/
