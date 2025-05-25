//===================================================================
//Functions for control of hardware of Calibrator Current Board (CCB)
//by Martin Praznovsky, 2025
//===================================================================


#ifndef CCB_H_
#define CCB_H_

#define DAC_CS					PORTB2
#define DAC_LDAC				PORTB1
#define DAC_CLR					PORTB0

#define RELAY_1_PORT			PORTD
#define RELAY_2_PORT			PORTD
#define RELAY_3_PORT			PORTD
#define RELAY_4_PORT			PORTC

#define RELAY_1S				PORTD5
#define RELAY_1R				PORTD4
#define RELAY_2S				PORTD7
#define RELAY_2R				PORTD6
#define RELAY_3S				PORTD3
#define RELAY_3R				PORTD2
#define RELAY_4S				PORTC3
#define RELAY_4R				PORTC2

#define ADR_DAC_DATA			0b00000001
#define ADR_CONFIG1				0b00000010
#define ADR_DAC_CLEAR_DATA		0b00000011
#define ADR_TRIGGER				0b00000100
#define ADR_STATUS				0b00000101
#define ADR_CONFIG2				0b00000110

#define CONFIG1					0b00000000010001010000
#define CONFIG2					0b00000000000000000011
#define TRIGGER					0b00000000000000000000

#define CS_HIGH()				(PORTB |= (1 << DAC_CS))
#define CS_LOW()				(PORTB &= ~(1 << DAC_CS))

#define LDAC_HIGH()				(PORTB |= (1 << DAC_LDAC))
#define LDAC_LOW()				(PORTB &= ~(1 << DAC_LDAC))

#define CLR_HIGH()				(PORTB |= (1<< DAC_CLR));
#define CLR_LOW()				(PORTB &= ~(1<< DAC_CLR));

#define OUT_LEDS_PORT			PORTC
#define OUT_LED_1				PORTC0
#define OUT_LED_2				PORTC1

#define LEDS_I2C_ADDRESS		56

/**
* @brief - initialize control pins for relays and puts each relay into default position (RESET)
* @returns - nothing
*/
void CCB_InitRelays(void);

/**
* @brief - set relay (switched position)
* @param relay - 
* @returns - nothing
*/
void CCB_RelaySET(uint8_t relay);

/**
* @brief - reset relay (default position)
* @returns - nothing
*/
void CCB_RelayRESET(uint8_t relay);

/**
* @brief - reset relay (default position)
* @returns - nothing
*/
void CCB_WriteDACRegister(uint8_t address, uint32_t data);

/**
* @brief - read content of DAC register on specified address
* @param address - address in the DAC memory
* @returns - content of the register on specified address
*/
uint32_t CCB_ReadDACRegister(uint8_t address);

/**
* @brief - initialize DAC11001B
* @returns - nothing
*/
void CCB_InitDAC(void);

/**
* @brief - set output voltage of the DAC
* @param code - 20-bit code for DAC
* @returns - nothing
*/
void CCB_SetDACVoltage(uint32_t code);

/**
* @brief - initialize front panel LEDs
* @returns - nothing
*/
void CCB_InitLEDs(void);

/**
* @brief - turn on/off front panel LEDs
* @returns - nothing
*/
void CCB_SetLEDs(uint8_t LEDs_vect);

#endif /* CCB_H_ */
