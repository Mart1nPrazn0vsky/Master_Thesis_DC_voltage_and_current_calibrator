#include <avr/io.h>
#include <util/twi.h>
#include "ATmega328P_I2CMaster.h"


#define I2C_STATUS_SUCCESS	0
#define I2C_STATUS_ERROR	1

#define I2C_WRITE		0
#define I2C_READ		1

#define I2C_Wait()		while (!(TWCR & (1 << TWINT)));


void I2C_Init(uint32_t bit_rate, unsigned long CLK_FREQ)
{
	bit_rate = (CLK_FREQ / bit_rate - 16) / 2 / 4;
	TWBR = bit_rate & 0xFF;
	TWSR = 0x00;				//reset register
	TWSR |= (1 << TWPS0);		//set baud rate prescaler to 4
}


uint8_t I2C_SendByte(uint8_t address, uint8_t data)
{
	uint8_t status = I2C_Start(address, I2C_WRITE);
	if (status) {return status;}
	
	status = I2C_Write(data);
	if (status) {return status;}
	
	I2C_Stop();
	
	return I2C_STATUS_SUCCESS;
}


uint8_t I2C_SendBytes(uint8_t address, uint8_t *data, uint8_t length)
{
	uint8_t status = I2C_Start(address, I2C_WRITE);		//start transmission
	if (status) {return status;}
	
	for (uint8_t i = 0; i < length; i++)
	{
		status = I2C_Write(data[i]);
		if (status) {return status;}
	}
	
	I2C_Stop();
	
	return I2C_STATUS_SUCCESS;
}


uint8_t I2C_ReadByte(uint8_t address, uint8_t data)
{
	uint8_t status = I2C_Start(address, I2C_READ);		//start transmission
	if (status) {return status;}
		
	data = I2C_ReadAck();
	
	I2C_Stop();
	
	return I2C_STATUS_SUCCESS;
}


uint8_t I2C_ReadBytes(uint8_t address, uint8_t *data, uint8_t length)
{
	uint8_t status = I2C_Start(address, I2C_READ);		//start transmission
	if (status) {return status;}
	
	for (uint8_t i = 0; i < (length - 1); i++)
	{
		data[i] = I2C_ReadAck();
	}
	
	data[length - 1] = I2C_ReadNack();		//read last byte without NACK
	
	I2C_Stop();
	
	return I2C_STATUS_SUCCESS;
}


uint8_t I2C_Start(uint8_t address, uint8_t mode)
{
	uint8_t twst = 0x00;
	TWCR = 0x00;		//reset register
	TWCR |= (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);	//clear TWINT flag, start bit, enable TWI
	
	I2C_Wait();		//wait for end of transmission
	
	//check if start condition was successfully transmitted
	twst = TW_STATUS & TW_STATUS_MASK;
	if ((twst != TW_START) && (twst != TW_REP_START))
	{
		return I2C_STATUS_ERROR;
	}
	
	TWDR = ((address << 1) | mode);			//load slave address into data register
	TWCR = (1 << TWINT) | (1 << TWEN);		//start transmission of address
	I2C_Wait();								//wait for end of transmission
	
	//check if slave has acknowledged the read/write mode
	twst = TW_STATUS & TW_STATUS_MASK;
	if ((twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK))
	{
		return I2C_STATUS_ERROR;
	}
	
	return I2C_STATUS_SUCCESS;
}


void I2C_Stop(void)
{
	TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);	
}


uint8_t I2C_Write(uint8_t data)
{
	uint8_t twst = 0x00;
	
	//put data into data register
	TWDR = data;
	
	TWCR = (1 << TWINT) | (1 << TWEN);		//start transmission of data
	I2C_Wait();								//wait for end of transmission
	
	//check if slave device acknowledged transmission
	twst = TW_STATUS & TW_STATUS_MASK;
	if (twst != TW_MT_DATA_ACK)
	{
		return I2C_STATUS_ERROR;
	}
	
	return I2C_STATUS_SUCCESS;
}


uint8_t I2C_ReadAck(void)
{
	//start TWI module and acknowledge data after reception
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	I2C_Wait();		//wait for end of transmission
	
	return TWDR;
}


uint8_t I2C_ReadNack(void)
{
	//start TWI module without acknowledging reception
	TWCR = (1 << TWINT) | (1 << TWEN);
	I2C_Wait();		//wait for end of transmission
	
	return TWDR;
}
