#include "Utils.h"


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
	string[i] = '\0';
}
