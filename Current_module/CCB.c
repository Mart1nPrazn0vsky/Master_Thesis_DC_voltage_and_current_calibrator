#define F_CPU		16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "CCB.h"


void CCB_InitRelays(void)
{
	DDRD |= (1 << DDD5) | (1 << DDD4);		//relay 1 control pins as output
	DDRD |= (1 << DDD7) | (1 << DDD6);		//relay 2 control pins as output
	DDRD |= (1 << DDD3) | (1 << DDD2);		//relay 3 control pins as output
	DDRC |= (1 << DDC3) | (1 << DDC2);		//relay 4 control pins as output
	
	RELAY_1_PORT &= ~(1 << RELAY_1S) & ~(1 << RELAY_1R);	//put relay in default position (RESET)
	RELAY_2_PORT &= ~(1 << RELAY_2S) & ~(1 << RELAY_2R);	//put relay in default position (RESET)
	RELAY_3_PORT &= ~(1 << RELAY_3S) & ~(1 << RELAY_3R);	//put relay in default position (RESET)
	RELAY_4_PORT &= ~(1 << RELAY_4S) & ~(1 << RELAY_4R);	//put relay in default position (RESET)
}


void CCB_RelaySET(uint8_t relay)
{
	if (relay == 1)
	{
		RELAY_1_PORT |= (1 << RELAY_1S);
		_delay_ms(10);
		RELAY_1_PORT &= ~(1 << RELAY_1S);
	}
	else if (relay == 2)
	{
		RELAY_2_PORT |= (1 << RELAY_2S);
		_delay_ms(10);
		RELAY_2_PORT &= ~(1 << RELAY_2S);
	}
	else if (relay == 3)
	{
		RELAY_3_PORT |= (1 << RELAY_3S);
		_delay_ms(10);
		RELAY_3_PORT &= ~(1 << RELAY_3S);
	}
	else if (relay == 4)
	{
		RELAY_4_PORT |= (1 << RELAY_4S);
		_delay_ms(10);
		RELAY_4_PORT &= ~(1 << RELAY_4S);
	}
}


void CCB_RelayRESET(uint8_t relay)
{
	if (relay == 1)
	{
		RELAY_1_PORT |= (1 << RELAY_1R);
		_delay_ms(10);
		RELAY_1_PORT &= ~(1 << RELAY_1R);
	}
	else if (relay == 2)
	{
		RELAY_2_PORT |= (1 << RELAY_2R);
		_delay_ms(10);
		RELAY_2_PORT &= ~(1 << RELAY_2R);
	}
	else if (relay == 3)
	{
		RELAY_3_PORT |= (1 << RELAY_3R);
		_delay_ms(10);
		RELAY_3_PORT &= ~(1 << RELAY_3R);
	}
	else if (relay == 4)
	{
		RELAY_4_PORT |= (1 << RELAY_4R);
		_delay_ms(10);
		RELAY_4_PORT &= ~(1 << RELAY_4R);
	}
}


void CCB_WriteDACRegister(uint8_t address, uint32_t data)
{
	CS_LOW();
	_delay_us(1);
	SPI_Transmit32(0x00000000 | (address << 24) | (data << 4));
	_delay_us(1);
	CS_HIGH();
}


uint32_t CCB_ReadDACRegister(uint8_t address)
{
	uint32_t data = 0x00000000;
	
	CS_LOW();
	data = SPI_Transmit32(0x80000000 | (address << 24) | (data << 4));
	CS_HIGH();
	
	return data;
}


void CCB_InitDAC(void)
{
	CCB_WriteDACRegister(ADR_CONFIG1, CONFIG1);
	_delay_us(1);
	CCB_WriteDACRegister(ADR_CONFIG2, CONFIG2);
	_delay_us(1);
	CCB_WriteDACRegister(ADR_TRIGGER, TRIGGER);
	_delay_us(1);
}


void CCB_SetDACVoltage(uint32_t code)
{
	CCB_WriteDACRegister(0x01, code);
	
	_delay_us(1);
	LDAC_LOW();
	_delay_us(1);
	LDAC_HIGH();	
}


void CCB_InitLEDs(void)
{
	DDRC |= (1 << DDC0) | (1 << DDC1);		//OUT_LED_1 and OUT_LED_2
	
	I2C_SendByte(LEDS_I2C_ADDRESS, 0x00);
}


void CCB_SetLEDs(uint8_t LEDs_vect)
{
	I2C_SendByte(LEDS_I2C_ADDRESS, LEDs_vect);
}
