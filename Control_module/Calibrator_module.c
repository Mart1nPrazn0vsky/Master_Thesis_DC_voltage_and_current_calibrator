#include "Calibrator_module.h"


volatile static uint32_t reg_G;
volatile static uint32_t reg_H;
volatile static uint32_t reg_I;
volatile static uint32_t reg_J;
volatile static uint32_t reg_K;
volatile static uint32_t reg_L;


uint8_t Module_WriteToRegister(UART* UART_handle, uint8_t reg_name, uint32_t data, uint8_t hex_size)
{
	uint8_t error = NO_ERROR;
	uint8_t string[50];
	uint32_t reg = 0x00000000;
	
	Utils_IntToHexString(data, string, hex_size);
	
	UART_SendByte(UART_handle, reg_name);		//send first letter (name of register)
	UART_SendString(UART_handle, string);		//send hexadecimal number in string format
	UART_SendByte(UART_handle, '\n');				//send end of message
	UART_SendByte(UART_handle, '\r');				//send end of message
	
	delay_ms(10);
	
	error = Module_ReadRegister(UART_handle, reg_name, &reg);						//get content of register
	if (error != NO_ERROR) {return error;}
	else if (reg != data) {error = ERROR_COMMUNICATION; return error;}	//check if register was written correctly
	
	return error;
}


uint8_t Module_ReadAllRegisters(UART* UART_handle)
{
	uint8_t error = NO_ERROR;
	uint8_t string[50];
	
	UART_ClearRXBuffer(UART_handle);						//clear RX buffer
	UART_SendString(UART_handle, "G003F\n\r");	//send '?' to get content of registers from module
	delay_ms(100);
	
	if (UART_AvailableBytes(UART_handle) > 0) {UART_ReadLine(UART_handle, string);}		//if data were received, read one line
	else {error = ERROR_COMMUNICATION; return error;}																	//if no data were received, return error
	
	if (string[0] == '@')		//check if name of module was received
	{
		while (UART_AvailableBytes(UART_handle) > 0)
		{
			UART_ReadLine(UART_handle, string);				//read one line
			if (string[0] == 'G') {Utils_RemoveCharFromString(string, 0); reg_G = Utils_HexStringToInt(string);}
			else if (string[0] == 'H') {Utils_RemoveCharFromString(string, 0); reg_H = Utils_HexStringToInt(string);}
			else if (string[0] == 'I') {Utils_RemoveCharFromString(string, 0); reg_I = Utils_HexStringToInt(string);}
			else if (string[0] == 'J') {Utils_RemoveCharFromString(string, 0); reg_J = Utils_HexStringToInt(string);}
			else if (string[0] == 'K') {Utils_RemoveCharFromString(string, 0); reg_K = Utils_HexStringToInt(string);}
			else if (string[0] == 'L') {Utils_RemoveCharFromString(string, 0); reg_L = Utils_HexStringToInt(string);}
			
			delay_ms(10);
		}
	}
	else {error = ERROR_COMMUNICATION; return error;}
	
	UART_ClearRXBuffer(UART_handle);			//clear RX buffer
	
	return error;
}


/*uint32_t Module_ReadRegister(UART* UART_handle, uint8_t reg_name)
{
	uint8_t error = NO_ERROR;
	uint32_t data = 0x00000000;
	
	error = Module_ReadAllRegisters(UART_handle);
	if (error != NO_ERROR) {return 0x00000000;}
		
	if (reg_name == 'G') {data = reg_G;}
	else if (reg_name == 'H') {data = reg_H;}
	else if (reg_name == 'I') {data = reg_I;}
	else if (reg_name == 'J') {data = reg_J;}
	else if (reg_name == 'K') {data = reg_K;}
	else if (reg_name == 'L') {data = reg_L;}
	
	return data;
}*/


uint8_t Module_ReadRegister(UART* UART_handle, uint8_t reg_name, uint32_t *data)
{
	uint8_t error = NO_ERROR;
	data = 0x00000000;
	
	error = Module_ReadAllRegisters(UART_handle);
	if (error != NO_ERROR) {return error;}
	else
	{
		if (reg_name == 'G') {*data = reg_G;}
		else if (reg_name == 'H') {*data = reg_H;}
		else if (reg_name == 'I') {*data = reg_I;}
		else if (reg_name == 'J') {*data = reg_J;}
		else if (reg_name == 'K') {*data = reg_K;}
		else if (reg_name == 'L') {*data = reg_L;}
	}
	
	return error;
}


uint8_t Module_GetName(UART *UART_handle, uint8_t *name)
{
	uint8_t error = NO_ERROR;
	
	UART_ClearRXBuffer(UART_handle);						//clear RX buffer
	UART_SendString(UART_handle, "G003F\n\r");	//send '?' to get content of registers from module
	delay_ms(100);
	
	if (UART_AvailableBytes(UART_handle) > 0) {UART_ReadLine(UART_handle, name);}		//if data were received, read one line
	else {error = ERROR_COMMUNICATION; return error;}																//if no data were received, return error
	
	if (name[0] == '@')		//check if name of module was received
	{
		delay_ms(100);										//wait until rest of data are received
		UART_ClearRXBuffer(UART_handle);	//clear RX buffer
	}
	else {error = ERROR_COMMUNICATION; return error;}
	
	return error;
}
