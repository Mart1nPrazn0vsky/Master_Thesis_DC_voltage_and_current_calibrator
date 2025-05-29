//==========================================================================================================
//Firmware for control of votlage and current calibrator
//Device consist of multiple PCB based modules:
//1. Voltage reference module (refered to as CVRB - Calibrator Voltage Reference Board)
//2. Low-voltage module (refered to as CLVB - Calibrator Low Voltage Board)
//3. Current module (refered to as CCB - Calibrator Current Board)
//Calibrator is controlled by user input through touchscreen display or via remote connection (USB/Ethernet)
//by Martin Praznovsky, 2025
//==========================================================================================================

#include "stm32f429xx.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "STM32F429ZI_SystemClock.h"
#include "STM32F429ZI_Delay.h"
#include "STM32F429ZI_GPIOPins.h"
#include "STM32F429ZI_UART.h"
#include "STM32F429ZI_SPIMaster.h"
#include "Lantronix_XPort.h"
#include "CLVB.h"
#include "CCB.h"
#include "Calibrator_errors.h"


void Calibrator_HandleRemoteControl(UART *UART_handle);
void Calibrator_ReadCommand(UART *UART_handle, uint8_t *command);
void Calibrator_HandleCommandFUNC(UART *UART_handle, uint8_t *command);
void Calibrator_HandleCommandVOLT(UART *UART_handle, uint8_t *command);
void Calibrator_HandleCommandCURR(UART *UART_handle, uint8_t *command);
void GetStateCLVB(void);
void GetStateCCB(void);


//ETHERNET variables
uint8_t string_Ethernet_IP[20];
uint8_t string_Ethernet_gateway[20];
uint8_t string_Ethernet_mask[20];
uint8_t string_Ethernet_DNS[20];

//CALIBRATOR MODULES
#define MODULE_NONE		0
#define MODULE_CLVB		1
#define MODULE_CCB		2
uint8_t module_selected = 0;

volatile static double CLVB_voltage = 0.0;			//desired value
volatile static double CLVB_frequency = 0.0;		//desired value
volatile static double CCB_current = 0.0;				//desired value

volatile static uint8_t error = NO_ERROR;

volatile static uint8_t byte = 0;
volatile static uint8_t CLVB_error = 0;
volatile static uint8_t CCB_error = 0;

volatile static uint8_t string[300];

CLVB_module_state CLVB_state_main;
CCB_module_state CCB_state_main;


int main(void)
{
	//init system clock
	SystemClockConfig(8000000, EXT_CLOCK);
	
	//init delays
	InitDelayTimer();
	
	delay_ms(100);
	
	//init all UARTs for USB, Ethernet, CLVB, CCB, CVRB
	UART *UART_USB = UART_Init(USART2, 9600, 45000000, 3, GPIOD, 5, GPIOD, 6);				//USB (FT232RN)
	UART *UART_ETHERNET = UART_Init(UART5, 9600, 45000000, 4, GPIOC, 12, GPIOD, 2);		//Ethernet (Lantronix XPort)
	UART *UART_CVRB = UART_Init(USART3, 9600, 45000000, 7, GPIOC, 10, GPIOC, 11);			//CVRB
	UART *UART_CLVB = UART_Init(USART6, 9600, 90000000, 5, GPIOG, 14, GPIOG, 9);			//CLVB
	UART *UART_CCB = UART_Init(UART7, 9600, 45000000, 6, GPIOE, 8, GPIOE, 7);					//CCB
	//UART *UART_SPARE = UART_Init(USART1, 9600, 90000000, 8, GPIOA, 10, GPIOA, 9);			//spare UART
	
	delay_ms(500);
	
	//init Lantronix XPort
	Lantronix_XPort_Init(UART_ETHERNET, 9600, GPIOG, 2);
	/*Lantronix_XPort_GetIPAddress(string_Ethernet_IP, string_Ethernet_gateway, string_Ethernet_mask, string_Ethernet_DNS);
	UART_SendString(UART_USB, "===================\n");
	UART_SendString(UART_USB, string_Ethernet_IP);
	UART_SendByte(UART_USB, '\n');
	UART_SendString(UART_USB, string_Ethernet_gateway);
	UART_SendByte(UART_USB, '\n');
	UART_SendString(UART_USB, string_Ethernet_mask);
	UART_SendByte(UART_USB, '\n');
	UART_SendString(UART_USB, string_Ethernet_DNS);
	UART_SendByte(UART_USB, '\n');
	UART_SendString(UART_USB, "===================\n");
	UART_ETHERNET = UART_Init(UART5, 9600, 45000000, 4, GPIOC, 12, GPIOD, 2);			//Ethernet (Lantronix XPort)*/
	
	
	//init SPI for display
	//-- will be added in next version, when calibrator is implemented in a box with display
	
	//init display
	//-- will be added in next version, when calibrator is implemented in a box with display
	
	
	//wait for few seconds for initialization of all modules
	delay_ms(3000);
	
	//init CLVB module
	error = Module_GetName(UART_CLVB, string);
	if (error == NO_ERROR) {UART_SendString(UART_USB, string);}
	error = CLVB_Init(UART_CLVB);
	if (error == NO_ERROR) {UART_SendString(UART_USB, "[CLVB NO_ERROR]\n");}
	else {UART_SendString(UART_USB, "[CLVB ERROR]\n");}
	Utils_ClearString(string);
	//CLVB_PrintRegisters(UART_USB);
	
	//delay_ms(1000);
	
	//init CCB module
	error = Module_GetName(UART_CCB, string);
	if (error == NO_ERROR) {UART_SendString(UART_USB, string);}
	error = CCB_Init(UART_CCB);
	if (error == NO_ERROR) {UART_SendString(UART_USB, "[CCB NO_ERROR]\n");}
	else {UART_SendString(UART_USB, "[CCB ERROR]\n");}
	Utils_ClearString(string);
	//CCB_PrintRegisters(UART_USB);
	
	delay_ms(1000);
	
	while (1)
	{
		//remote control via USB
		if (UART_AvailableBytes(UART_USB) > 0)
		{
			/*byte = UART_ReadByte(UART_USB);
			UART_SendByte(UART_USB, byte);*/
			
			delay_ms(10);
			Calibrator_HandleRemoteControl(UART_USB);
		}
		
		//remote control via Ethernet
		if (UART_AvailableBytes(UART_ETHERNET) > 0)
		{
			/*byte = UART_ReadByte(UART_ETHERNET);
			UART_SendByte(UART_USB, byte);*/
			
			delay_ms(10);
			Calibrator_HandleRemoteControl(UART_ETHERNET);
		}
		
		//control via touchscreen display
		//-- will be added in next version, when calibrator is implemented in a box with display
	}
	
	return 0;
}


void Calibrator_ReadCommand(UART *UART_handle, uint8_t *command)
{
	error = NO_ERROR;
	
	uint8_t time_counter = 0;
	uint8_t i = 0;
	uint8_t c = 0;
	uint8_t end = 0;
	
	uint8_t str[10];
	
	//read line manually with timeout in case user did not send '\n' at the end of string
	while (1)
	{
		while (UART_AvailableBytes(UART_handle) > 0)
		{
			c = UART_ReadByte(UART_handle);
			if ((c >= 'a') && (c <= 'z')) {c -= 32;}		//convert lowercase letters to capital letters
			command[i] = c;
			i++;
			if ((c == '\n') || (c == '\r')) {command[i-1] = '\0'; end = 1; break;}		//finish string, remove '\n', break inner loop
		}
		if (end == 1) {break;}		//line was received, break outer loop
		delay_ms(10);
		time_counter++;
		if (time_counter > 100)		//timeout
		{
			error = ERROR_USER_INPUT;
			return;
		}	
	}
}


void Calibrator_HandleRemoteControl(UART *UART_handle)
{
	//1. read command
	//2. check if command starts with FUNC, VOLT or CURR
	//3. execute command
	//4. if error occured, print error message
	
	error = NO_ERROR;
	uint8_t command[50];
	
	if (UART_AvailableBytes(UART_handle) > 0)
	{
		Calibrator_ReadCommand(UART_handle, command);		//read one command from USB/Ethernet RX buffer
	}
	
	//if command was received without error, handle it
	if (error == NO_ERROR)
	{
		//if command is not empty string
		if (strlen(command) > 0)
		{
			//switching modules
			if (Utils_CheckForSubstring(command, "FUNC"))
			{
				Calibrator_HandleCommandFUNC(UART_handle, command);
			}
			//voltage control
			else if (Utils_CheckForSubstring(command, "VOLT"))
			{
				Calibrator_HandleCommandVOLT(UART_handle, command);
			}
			//current control
			else if (Utils_CheckForSubstring(command, "CURR"))
			{
				Calibrator_HandleCommandCURR(UART_handle, command);
			}
			//any other "command"
			else
			{
				error = ERROR_UNKNOWN_COMMAND;
			}
		}
	}
	
	//print error messages if necessary
	if (error == ERROR_USER_INPUT) {UART_SendString(UART_handle, "ERROR: Wrong input.\n\r");}
	else if (error == ERROR_UNKNOWN_COMMAND) {UART_SendString(UART_handle, "ERROR: Unknown command.\n\r");}
	else if (error == ERROR_COMMUNICATION) {UART_SendString(UART_handle, "ERROR: Unsuccessful communication with module (internal problem).\n\r");}
	else if (error == ERROR_WRONG_MODULE) {UART_SendString(UART_handle, "ERROR: Wrong module is connected to UART line (internal problem).\n\r");}
	else if (error == ERROR_VOLT_NOT_SELECTED) {UART_SendString(UART_handle, "ERROR: Voltage module is not selected.\n\r");}
	else if (error == ERROR_CURR_NOT_SELECTED) {UART_SendString(UART_handle, "ERROR: Current module is not selected.\n\r");}
	else if (error == ERROR_VOLT_RANGE) {UART_SendString(UART_handle, "ERROR: Voltage is out of range.\n\r");}
	else if (error == ERROR_CURR_RANGE) {UART_SendString(UART_handle, "ERROR: Current is out of range.\n\r");}
	else if (error == ERROR_FREQ_RANGE) {UART_SendString(UART_handle, "ERROR: Frequency is out of range.\n\r");}
	else if (error == ERROR_NONEXISTENT_RANGE) {UART_SendString(UART_handle, "ERROR: Requested range does not exist.\n\r");}
}


void Calibrator_HandleCommandFUNC(UART *UART_handle, uint8_t *command)
{
	//================================================
	if (Utils_CheckForSubstring(command, "FUNC VOLT"))				//switch to CLVB module
	{
		//CCB_TurnOFFModule();
		module_selected = MODULE_CLVB;
	}
	//=====================================================
	else if (Utils_CheckForSubstring(command, "FUNC CURR"))		//switch to CBB module
	{
		//CLVB_TurnOFFModule();
		module_selected = MODULE_CCB;
	}
	//=================================================
	else if (Utils_CheckForSubstring(command, "FUNC?"))				//respond with current module name
	{
		if (module_selected == MODULE_NONE) {UART_SendString(UART_handle, "NONE\n\r");}
		else if (module_selected == MODULE_CLVB) {UART_SendString(UART_handle, "VOLT\n\r");}
		else if (module_selected == MODULE_CCB) {UART_SendString(UART_handle, "CURR\n\r");}
	}
	else
	{
		error = ERROR_USER_INPUT;
	}
}


void Calibrator_HandleCommandVOLT(UART *UART_handle, uint8_t *command)
{
	//if voltage module is selected
	if (module_selected == MODULE_CLVB)
	{
		//============================================
		if (Utils_CheckForSubstring(command, "VOLT "))									//read number and set voltage (check ranges etc.)
		{
			if (sscanf(command, "VOLT %lf", &CLVB_voltage) != 1) {error = ERROR_USER_INPUT;}	//function should return number of read numbers (1)
			else
			{
				if (CLVB_state_main.mode == CLVB_MODE_DC) {error = CLVB_SetVoltageDC(CLVB_voltage);}		//DC mode
				else {error = CLVB_SetVoltageAC(CLVB_voltage, CLVB_frequency);}													//AC mode
			}
		}
		//=================================================
		else if (Utils_CheckForSubstring(command, "VOLT?"))									//send string with selected voltage
		{
			if (CLVB_state_main.range == 1) {sprintf(string, "%.7f V\n\r", CLVB_state_main.voltage);}
			else if (CLVB_state_main.range == 2) {sprintf(string, "%.6f V\n\r", CLVB_state_main.voltage);}
			else if (CLVB_state_main.range == 3) {sprintf(string, "%.5f V\n\r", CLVB_state_main.voltage);}
			UART_SendString(UART_handle, string);
		}
		//======================================================
		else if (Utils_CheckForSubstring(command, "VOLT:FREQ "))						//read number and set voltage (check ranges etc.)
		{
			if (sscanf(command, "VOLT:FREQ %lf", &CLVB_frequency) != 1) {error = ERROR_USER_INPUT;}			//function should return number of read numbers (1)
			else {error = CLVB_SetFrequency(CLVB_frequency);}
		}
		//======================================================
		else if (Utils_CheckForSubstring(command, "VOLT:FREQ?"))						//read number and set voltage (check ranges etc.)
		{
			sprintf(string, "%.7f Hz\n\r", CLVB_state_main.frequency);
			UART_SendString(UART_handle, string);
		}
		//======================================================
		else if (Utils_CheckForSubstring(command, "VOLT:RANG "))			//read number and range
		{
			uint8_t range = 0;
			if (sscanf(command, "VOLT:RANG %d", &range) != 1) {error = ERROR_USER_INPUT;}			//function should return number of read numbers (1)
			else {error = CLVB_SetRange(range);}
		}
		//======================================================
		else if (Utils_CheckForSubstring(command, "VOLT:RANG?"))						//send string with voltage range
		{
			sprintf(string, "%d\n\r", CLVB_state_main.range);
			UART_SendString(UART_handle, string);
		}
		//=============================================================
		else if (Utils_CheckForSubstring(command, "VOLT:RANG:AUTO ON"))			//set voltage autorange to ON
		{
			CLVB_AutorangeON();
		}
		//==============================================================
		else if (Utils_CheckForSubstring(command, "VOLT:RANG:AUTO OFF"))		//set voltage autorange to OFF
		{
			CLVB_AutorangeOFF();
		}
		//===========================================================
		else if (Utils_CheckForSubstring(command, "VOLT:RANG:AUTO?"))			//send string with voltage range
		{
			if (CLVB_state_main.autorange_state == CLVB_AUTORANGE_ON) {UART_SendString(UART_handle, "Autorange is ON.\n\r");}
			else {UART_SendString(UART_handle, "Autorange is OFF.\n\r");}
		}
		//========================================================
		else if (Utils_CheckForSubstring(command, "VOLT:MODE DC"))					//set voltage mode to DC
		{
			error = CLVB_SetVoltageDC(CLVB_state_main.voltage);
		}
		//========================================================
		else if (Utils_CheckForSubstring(command, "VOLT:MODE AC"))					//set voltage mode to AC
		{
			error = CLVB_SetVoltageAC(CLVB_state_main.voltage, CLVB_state_main.frequency);
		}
		//======================================================
		else if (Utils_CheckForSubstring(command, "VOLT:MODE?"))						//send string with selected voltage mode
		{
			if (CLVB_state_main.mode == CLVB_MODE_DC) {UART_SendString(UART_handle, "DC mode.\n\r");}
			else {UART_SendString(UART_handle, "AC mode.\n\r");}
		}
		//========================================================
		else if (Utils_CheckForSubstring(command, "VOLT:OUTP ON"))					//turn ON voltage output
		{
			error = CLVB_OutputON();
		}
		//=========================================================
		else if (Utils_CheckForSubstring(command, "VOLT:OUTP OFF"))					//turn OFF voltage output
		{
			error = CLVB_OutputOFF();
		}
		//======================================================
		else if (Utils_CheckForSubstring(command, "VOLT:OUTP?"))						//send string with information if output is ON or OFF
		{
			if (CLVB_state_main.output_state == CLVB_OUTPUT_ON) {UART_SendString(UART_handle, "Output ON.\n\r");}
			else {UART_SendString(UART_handle, "Output OFF.\n\r");}
		}
		else
		{
			error = ERROR_UNKNOWN_COMMAND;
		}
			
		GetStateCLVB();		//update everything
	}
	//if voltage module is not selected
	else
	{
		//UART_SendString(UART_handle, "ERROR: Voltage module is not selected.\n\r");
		error = ERROR_VOLT_NOT_SELECTED;
	}
}



void Calibrator_HandleCommandCURR(UART *UART_handle, uint8_t *command)
{
	//if current module is selected
	if (module_selected == MODULE_CCB)
	{
		//============================================
		if (Utils_CheckForSubstring(command, "CURR "))									//read number and set current (check ranges etc.)
		{
			if (sscanf(command, "CURR %lf", &CCB_current) != 1) {error = ERROR_USER_INPUT;}	//function should return number of read numbers (1)
			else {error = CCB_SetCurrent(CCB_current);}
		}
		//=================================================
		else if (Utils_CheckForSubstring(command, "CURR?"))									//send string with selected voltage
		{
			if (CLVB_state_main.range == 1) {sprintf(string, "%.7f V\n\r", CCB_state_main.current);}
			else if (CLVB_state_main.range == 2) {sprintf(string, "%.6f V\n\r", CCB_state_main.current);}
			else if (CLVB_state_main.range == 3) {sprintf(string, "%.5f V\n\r", CCB_state_main.current);}
			UART_SendString(UART_handle, string);
		}
		//======================================================
		else if (Utils_CheckForSubstring(command, "CURR:RANG "))			//read number and range
		{
			uint8_t range = 0;
			if (sscanf(command, "CURR:RANG %d", &range) != 1) {error = ERROR_USER_INPUT;}			//function should return number of read numbers (1)
			else {error = CCB_SetRange(range);}
		}
		//======================================================
		else if (Utils_CheckForSubstring(command, "CURR:RANG?"))						//send string with voltage range
		{
			sprintf(string, "%d\n\r", CCB_state_main.range);
			UART_SendString(UART_handle, string);
		}
		//=============================================================
		else if (Utils_CheckForSubstring(command, "CURR:RANG:AUTO ON"))			//set voltage autorange to ON
		{
			CCB_AutorangeON();
		}
		//==============================================================
		else if (Utils_CheckForSubstring(command, "CURR:RANG:AUTO OFF"))		//set voltage autorange to OFF
		{
			CCB_AutorangeOFF();
		}
		//===========================================================
		else if (Utils_CheckForSubstring(command, "CURR:RANG:AUTO?"))			//send string with voltage range
		{
			if (CLVB_state_main.autorange_state == CCB_AUTORANGE_ON) {UART_SendString(UART_handle, "Autorange is ON.\n\r");}
			else {UART_SendString(UART_handle, "Autorange is OFF.\n\r");}
		}
		//========================================================
		else if (Utils_CheckForSubstring(command, "CURR:OUTP ON"))					//turn ON voltage output
		{
			error = CCB_OutputON();
		}
		//=========================================================
		else if (Utils_CheckForSubstring(command, "CURR:OUTP OFF"))					//turn OFF voltage output
		{
			error = CCB_OutputOFF();
		}
		//======================================================
		else if (Utils_CheckForSubstring(command, "CURR:OUTP?"))						//send string with information if output is ON or OFF
		{
			if (CCB_state_main.output_state == CCB_OUTPUT_ON) {UART_SendString(UART_handle, "Output ON.\n\r");}
			else {UART_SendString(UART_handle, "Output OFF.\n\r");}
		}
		else
		{
			error = ERROR_UNKNOWN_COMMAND;
		}
			
		GetStateCCB();		//update everything
	}
	//if current module is not selected
	else
	{
		//UART_SendString(UART_handle, "ERROR: Current module is not selected.\n\r");
		error = ERROR_CURR_NOT_SELECTED;
	}
}


void GetStateCLVB(void)
{	
	CLVB_state_main.voltage = CLVB_GetVoltage();
	CLVB_state_main.frequency = CLVB_GetFrequency();
	CLVB_state_main.range = CLVB_GetRange();
	CLVB_state_main.mode = CLVB_GetMode();
	CLVB_state_main.output_state = CLVB_GetOutputState();
	CLVB_state_main.autorange_state = CLVB_GetAutorangeState();
	CLVB_state_main.dithering_state = CLVB_GetDitheringState();
}


void GetStateCCB(void)
{	
	CCB_state_main.current = CCB_GetCurrent();
	CCB_state_main.range = CCB_GetRange();
	CCB_state_main.output_state = CCB_GetOutputState();
	CCB_state_main.autorange_state = CCB_GetAutorangeState();
	CCB_state_main.dithering_state = CCB_GetDitheringState();
}
