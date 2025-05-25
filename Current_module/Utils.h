//====================================================
//Utility functions for Calibrator Current Board (CCB)
//by Martin Praznovsky, 2025
//====================================================

#include <stdint.h>


#ifndef UTILS_H_
#define UTILS_H_

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
* @brief - remove one character on specified position from string
* @param string - string
* @param index - position of character to be removed
* @returns - nothing
*/
void Utils_RemoveCharFromString(uint8_t *string, uint8_t index);

#endif /* UTILS_H_ */
