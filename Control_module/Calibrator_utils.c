#include "Calibrator_utils.h"


uint32_t Utils_HexStringToInt(uint8_t *string)
{
	uint32_t number = 0;
	uint8_t byte = 0;
	uint8_t i = 0;
	
	while (string[i] != '\0')
	{
		if ((string[i] >= '0') && (string[i] <= '9')) {byte = (string[i] - '0');}
		else if ((string[i] >= 'A') && (string[i] <= 'F')) {byte = (string[i] - 'A' + 10);}
		
		number = (number << 4) | (byte & 0x0F);
		i++;
	}

	return number;
}


void Utils_IntToHexString(uint32_t number, uint8_t *string, uint8_t length)
{
	uint8_t byte = 0;
	
	for (uint8_t i = 0; i < length; i++)
	{
		byte = (number >> (4 * i)) & 0x0F;
		if ((byte >= 0) && (byte <= 9)) {string[length - i - 1] = byte + '0';}
		else if ((byte >= 10) && (byte <= 15)) {string[length - i - 1] = byte + 'A' - 10;}
	}
	
	string[length] = '\0';
}


void Utils_RemoveCharFromString(uint8_t *string, uint8_t index)
{
    uint8_t i = index;
    
    while (string[i] != '\0')
    {
        string[i] = string[i + 1];
        i++;
    }
}


void Utils_ClearString(uint8_t *string)
{
	uint8_t i = 0;
	
	while(string[i] != '\0')
	{
		string[i] = '\0';
		i++;
	}
}


void Utils_AppendString(uint8_t *string_1, uint8_t *string_2)
{
	uint8_t i = 0;
	uint8_t j = 0;
	
	while (string_1[i] != '\0') {i++;}		//get to last index of string_1
	while (string_2[j] != '\0')
	{
		string_1[i] = string_2[j];
		i++;
		j++;
	}
}


uint8_t Utils_CheckForSubstring(uint8_t *string, uint8_t *substring)
{
	uint8_t result = 1;
	uint8_t i = 0;
	
	while (substring[i] != '\0')
	{
	    if (substring[i] != string[i]) {result = 0; break;}		//if characters at same index are different, return 0
	    i++;
	}
	
	return result;
}


uint32_t Utils_SetBit(uint32_t reg, uint8_t bit)
{
	reg |= (1 << bit);
	return reg;
}


uint32_t Utils_ClearBit(uint32_t reg, uint8_t bit)
{
	reg &= ~(1 << bit);
	return reg;
}


uint8_t Utils_GetBit(uint32_t reg, uint8_t bit)
{
	return ((reg >> bit) & 0x00000001);
}
