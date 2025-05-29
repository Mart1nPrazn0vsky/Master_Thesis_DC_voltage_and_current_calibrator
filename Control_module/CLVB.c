#include "CLVB.h"


#define G							71
#define H							72
#define I							73
#define J							74
#define REG_G_SIZE		4
#define REG_H_SIZE		4
#define REG_I_SIZE		8
#define REG_J_SIZE		8

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

#define LED_OFF				0
#define LED_ON				1

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

volatile static uint16_t register_G = 0x0000;
volatile static uint16_t register_H = 0x0000;
volatile static uint32_t register_I = 0x00000000;
volatile static uint32_t register_J = 0x00000000;

UART* UART_CLVB;
CLVB_module_state CLVB_state;


uint8_t CLVB_Init(UART *UART_handle)
{
	uint8_t error = NO_ERROR;
	UART_CLVB = UART_handle;
	uint8_t string[10];
	
	delay_ms(100);
	
	for (uint8_t i = 0; i < 3; i++)				//try connect to CLVB module 3 times
	{
		error = Module_GetName(UART_CLVB, string);
		if (error == NO_ERROR) {break;}			//if no error occured, end this loop
		delay_ms(1000);											//delay between attempts
	}
	
	if (error == NO_ERROR)
	{
		if (strcmp(string, "@CLVB") != 0)		//if strings are same, strcmp returns 0
		{
			error = ERROR_WRONG_MODULE;
			return error;
		}
	}
	else
	{
		return error;
	}
	
	error = CLVB_SetRange(3);
	error = CLVB_SetVoltageDC(0.0);
	
	//turn on/off LEDs
	
	CLVB_state.voltage = 0.0;
	CLVB_state.frequency = 0.0;
	CLVB_state.range = 3;
	CLVB_state.mode = CLVB_MODE_DC;
	CLVB_state.output_state = CLVB_OUTPUT_OFF;
	CLVB_state.autorange_state = CLVB_AUTORANGE_OFF;
	CLVB_state.dithering_state = CLVB_DITHERING_OFF;
	
	return error;
}


uint8_t CLVB_TurnOFFModule(void)
{
	uint8_t error;
	
	error = CLVB_SetVoltageDC(0.0);
	error = CLVB_OutputOFF();
	
	return error;
}


uint8_t CLVB_SetRange(uint8_t range)
{
	uint8_t error = NO_ERROR;
	
	if ((range < 1) || (range > 3)) {error = ERROR_NONEXISTENT_RANGE; return error;}
	
	if (range == 1)
	{
		register_H = Utils_SetBit(register_H, K2);
		register_H = Utils_SetBit(register_H, K3);
	}
	else if (range == 2)
	{
		register_H = Utils_ClearBit(register_H, K2);
		register_H = Utils_SetBit(register_H, K3);
	}
	else if (range == 3)
	{
		register_H = Utils_ClearBit(register_H, K2);
		register_H = Utils_ClearBit(register_H, K3);
	}
	
	error = Module_WriteToRegister(UART_CLVB, H, register_H, REG_H_SIZE);
	if (error == NO_ERROR) {CLVB_state.range = range;}
	
	return error;
}


uint8_t CLVB_CheckRange(double voltage)
{
	uint8_t error = NO_ERROR;
	
	if ((CLVB_state.range == 1) && ((voltage > CLVB_R1_max) || (voltage < CLVB_R1_min))) {error = ERROR_VOLT_RANGE;}
	else if ((CLVB_state.range == 2) && ((voltage > CLVB_R2_max) || (voltage < CLVB_R2_min))) {error = ERROR_VOLT_RANGE;}
	else if ((CLVB_state.range == 3) && ((voltage > CLVB_R3_max) || (voltage < CLVB_R3_min))) {error = ERROR_VOLT_RANGE;}
	
	return error;
}


uint8_t CLVB_Autorange(double voltage)
{
	uint8_t error = NO_ERROR;
	
	if ((voltage <= CLVB_R1_max) && (voltage >= CLVB_R1_min)) {error = CLVB_SetRange(1);}
	else if ((voltage <= CLVB_R2_max) && (voltage >= CLVB_R2_min)) {error = CLVB_SetRange(2);}
	else if ((voltage <= CLVB_R3_max) && (voltage >= CLVB_R3_min)) {error = CLVB_SetRange(3);}
	else {error = ERROR_VOLT_RANGE;}
	
	return error;
}


void CLVB_AutorangeON(void)
{
	CLVB_state.autorange_state = CLVB_AUTORANGE_ON;
}

void CLVB_AutorangeOFF(void)
{
	CLVB_state.autorange_state = CLVB_AUTORANGE_OFF;
}

uint8_t CLVB_OutputON(void)
{
	uint8_t error = NO_ERROR;
	
	register_H = Utils_SetBit(register_H, K1);
	error = Module_WriteToRegister(UART_CLVB, H, register_H, REG_H_SIZE);
	if (error == NO_ERROR) {CLVB_state.output_state = CLVB_OUTPUT_ON;}
	
	return error;
}


uint8_t CLVB_OutputOFF(void)
{
	uint8_t error = NO_ERROR;
	
	register_H = Utils_ClearBit(register_H, K1);
	error = Module_WriteToRegister(UART_CLVB, H, register_H, REG_H_SIZE);
	if (error == NO_ERROR) {CLVB_state.output_state = CLVB_OUTPUT_OFF;}
	
	return error;
}


uint8_t CLVB_DitheringON(void)
{
	uint8_t error = NO_ERROR;
	
	register_H = Utils_SetBit(register_H, DIT);
	error = Module_WriteToRegister(UART_CLVB, H, register_H, REG_H_SIZE);
	if (error == NO_ERROR) {CLVB_state.dithering_state = CLVB_DITHERING_ON;}
	
	return error;
}


uint8_t CLVB_DitheringOFF(void)
{
	uint8_t error = NO_ERROR;
	
	register_H = Utils_ClearBit(register_H, DIT);
	error = Module_WriteToRegister(UART_CLVB, H, register_H, REG_H_SIZE);
	if (error == NO_ERROR) {CLVB_state.dithering_state = CLVB_DITHERING_OFF;}
	
	return error;
}


uint8_t CLVB_SetVoltageDC(double voltage)
{
	uint8_t error = NO_ERROR;
	
	//handle range
	if (CLVB_state.autorange_state == CLVB_AUTORANGE_OFF)
	{
		error = CLVB_CheckRange(voltage);				//if autorange is OFF, check if voltage is within selected range
	}
	else {error = CLVB_Autorange(voltage);}		//if autorange is ON, find correct range and set it
	if (error != NO_ERROR) {return error;}		//in case of any problem, return error
	
	//handle mode
	register_H = Utils_ClearBit(register_H, AC);														//turn off AC mode
	error = Module_WriteToRegister(UART_CLVB, H, register_H, REG_H_SIZE);		//update register H
	if (error != NO_ERROR) {return error;}		//in case of any problem, return error
	else {CLVB_state.mode = CLVB_MODE_DC;}
	
	//handle voltage
	if (CLVB_state.dithering_state == CLVB_DITHERING_OFF)
	{
		register_I = (CLVB_GetVoltageCode(voltage) << 4);											//update register I without dithering
	}
	else {register_I = CLVB_GetVoltageCode(voltage);}												//update register I
	error = Module_WriteToRegister(UART_CLVB, I, register_I, REG_I_SIZE);		//update register I
	if (error != NO_ERROR) {return error;}		//in case of any problem, return error
	else {CLVB_state.voltage = voltage;}
	
	return error;
}


uint8_t CLVB_SetVoltageAC(double voltage, double frequency)
{
	uint8_t error = NO_ERROR;
	
	//handle range
	if (CLVB_state.autorange_state == CLVB_AUTORANGE_OFF)
	{
		error = CLVB_CheckRange(voltage);				//if autorange is OFF, check if voltage is within selected range
	}
	else {error = CLVB_Autorange(voltage);}		//if autorange is ON, find correct range and set it
	if (error != NO_ERROR) {return error;}		//in case of any problem, return error
	
	//handle frequency
	error = CLVB_SetFrequency(frequency);			//set frequency
	if (error != NO_ERROR) {return error;}		//in case of any problem, return error
	
	//handle mode
	register_H = Utils_SetBit(register_H, AC);														//turn off AC mode
	error = Module_WriteToRegister(UART_CLVB, H, register_H, REG_H_SIZE);		//update register H
	if (error != NO_ERROR) {return error;}		//in case of any problem, return error
	else {CLVB_state.mode = CLVB_MODE_AC;}
	
	//handle voltage
	register_I = (CLVB_GetVoltageCode(voltage) << 4);			//when generating AC voltage, no dithering is applied
	error = Module_WriteToRegister(UART_CLVB, I, register_I, REG_I_SIZE);		//update register I
	if (error != NO_ERROR) {return error;}		//in case of any problem, return error
	else {CLVB_state.voltage = voltage;}
	
	return error;
}


uint32_t CLVB_GetVoltageCode(double voltage)
{
	uint32_t code = 0x00000000;
	
	if ((CLVB_state.dithering_state == 0) || (CLVB_state.mode == CLVB_MODE_AC))		//if dithering is off or when generating AC signal
	{
		if (CLVB_state.range == 1)
		{
			code = (uint32_t) (((voltage - CLVB_R1_offset_error) / (CLVB_R1_gain * CLVB_R1_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution / (CLVB_VREFPF - CLVB_VREFNF)));
		}
		else if (CLVB_state.range == 2)
		{
			code = (uint32_t) (((voltage - CLVB_R2_offset_error) / (CLVB_R2_gain * CLVB_R2_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution / (CLVB_VREFPF - CLVB_VREFNF)));
		}
		else if (CLVB_state.range == 3)
		{
			code = (uint32_t) (((voltage - CLVB_R3_offset_error) / (CLVB_R3_gain * CLVB_R3_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution / (CLVB_VREFPF - CLVB_VREFNF)));
		}
	}
	else		//if dithering is on
	{
		if (CLVB_state.range == 1)
		{
			code = (uint32_t) (((voltage - CLVB_R1_offset_error) / (CLVB_R1_gain * CLVB_R1_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution_dith / (CLVB_VREFPF - CLVB_VREFNF)));
		}
		else if (CLVB_state.range == 2)
		{
			code = (uint32_t) (((voltage - CLVB_R2_offset_error) / (CLVB_R2_gain * CLVB_R2_gain_error) - CLVB_VREFNF) * (CLVB_DAC_resolution_dith / (CLVB_VREFPF - CLVB_VREFNF)));
		}
		else if (CLVB_state.range == 3)
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
	error = Module_WriteToRegister(UART_CLVB, J, register_J, REG_J_SIZE);		//update register J
	if (error != NO_ERROR) {return error;}
	else {CLVB_state.frequency = frequency;}
	
	return error;
}


uint8_t CLVB_SetLED(uint8_t led, uint8_t value)
{
	uint8_t error = NO_ERROR;
	
	if (value == LED_OFF) {register_H = Utils_ClearBit(register_H, led);}		//turn OFF LED
	else {register_H = Utils_SetBit(register_H, led);}											//turn ON LED
	error = Module_WriteToRegister(UART_CLVB, H, register_H, REG_H_SIZE);
	
	return error;
}


double CLVB_GetVoltage(void)
{
	return CLVB_state.voltage;
}


double CLVB_GetFrequency(void)
{
	return CLVB_state.frequency;
}


uint8_t CLVB_GetRange(void)
{
	return CLVB_state.range;
}


uint8_t CLVB_GetAutorangeState(void)
{
	return CLVB_state.autorange_state;
}


uint8_t CLVB_GetOutputState(void)
{
	return CLVB_state.output_state;
}


uint8_t CLVB_GetDitheringState(void)
{
	return CLVB_state.dithering_state;
}


uint8_t CLVB_GetMode(void)
{
	return CLVB_state.mode;
}


uint8_t CLVB_PrintRegisters(UART *UART_USB)
{
	uint8_t error = NO_ERROR;
	uint32_t data;
	
	error = Module_ReadRegister(UART_CLVB, G, &data);
	if (error != NO_ERROR) {return error;}
	register_G = data;
	error = Module_ReadRegister(UART_CLVB, H, &data);
	if (error != NO_ERROR) {return error;}
	register_H = data;
	error = Module_ReadRegister(UART_CLVB, I, &data);
	if (error != NO_ERROR) {return error;}
	register_I = data;
	error = Module_ReadRegister(UART_CLVB, J, &data);
	if (error != NO_ERROR) {return error;}
	register_J = data;
	
	uint8_t string_G[20];
	uint8_t string_H[20];
	uint8_t string_I[20];
	uint8_t string_J[20];
	
	Utils_IntToHexString(register_G, string_G, 4);
	Utils_IntToHexString(register_H, string_H, 4);
	Utils_IntToHexString(register_I, string_I, 8);
	Utils_IntToHexString(register_J, string_J, 8);
	
	UART_SendString(UART_USB, "CLVB:\n");
	UART_SendString(UART_USB, "G: ");
	UART_SendString(UART_USB, string_G);
	UART_SendString(UART_USB, "\n");
	UART_SendString(UART_USB, "H: ");
	UART_SendString(UART_USB, string_H);
	UART_SendString(UART_USB, "\n");
	UART_SendString(UART_USB, "I: ");
	UART_SendString(UART_USB, string_I);
	UART_SendString(UART_USB, "\n");
	UART_SendString(UART_USB, "J: ");
	UART_SendString(UART_USB, string_J);
	UART_SendString(UART_USB, "\n");
	
	return error;
}