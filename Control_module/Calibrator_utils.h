#include <stdint.h>


#ifndef CALIBRATOR_UTILS_H_
#define CALIBRATOR_UTILS_H_

/**
* @brief - convert hexadecimal number in string format to integer
* @param string - hexadecimal string to be converted
* @returns - integer
*/
uint32_t Utils_HexStringToInt(uint8_t *string);

/**
* @brief - convert integer to hexadecimal number in string format
* @param number - integer to be converted
* @param string - string for storing output
* @param length - length of output string ("7F" with length 4 is "007F")
* @returns - nothing
*/
void Utils_IntToHexString(uint32_t number, uint8_t *string, uint8_t length);

/**
* @brief - remove character on specified position from string
* @param string - string for storing output
* @param index - position of character to be removed
* @returns - nothing
*/
void Utils_RemoveCharFromString(uint8_t *string, uint8_t index);

/**
* @brief - remove  all characters from string
* @param string - string to be cleared
* @returns - nothing
*/
void Utils_ClearString(uint8_t *string);

/**
* @brief - connect second string to the first string
* @param string_1 - first string
* @param string_2 - second string
* @returns - nothing
*/
void Utils_AppendString(uint8_t *string_1, uint8_t *string_2);

/**
* @brief - check if string contains substring (starting from index 0)
* @param string - string to be checked
* @param substring - substring to look for
* @returns - 1 is string contains substring, 0 if not
*/
uint8_t Utils_CheckForSubstring(uint8_t *string, uint8_t *substring);

/**
* @brief - set specified bit in register (binary number) to 1
* @param reg - register
* @param bit - specified bit
* @returns - register with specified bit set to 1
*/
uint32_t Utils_SetBit(uint32_t reg, uint8_t bit);

/**
* @brief - set specified bit in register (binary number) to 0
* @param reg - register
* @param bit - specified bit
* @returns - register with specified bit set to 0
*/
uint32_t Utils_ClearBit(uint32_t reg, uint8_t bit);

/**
* @brief - read status of specified bit in register (binary number)
* @param reg - register
* @param bit - specified bit
* @returns - value of bit in register (0 or 1)
*/
uint8_t Utils_GetBit(uint32_t reg, uint8_t bit);

#endif
