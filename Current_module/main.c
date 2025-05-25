//=========================================================
//Main script for control of Calibrator Current Board (CCB)
//by Martin Praznovsky, 2025
//=========================================================


#define F_CPU		16000000UL

#include <avr/io.h>
#include <avr/fuse.h>
#include <avr/lock.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "ATmega328P_UART.h"
#include "ATmega328P_SPIMaster.h"
#include "ATmega328P_I2CMaster.h"
#include "Utils.h"
#include "CCB.h"


//FUSE SETTINGS
FUSES = {
	.low = 0xFF,
	.high = 0xDA,
	.extended = 0xFD,
};

//LOCKBITS
LOCKBITS = 0x3F;


//  register G
//	-----------------------------------------------------------------
//  |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
//  -----------------------------------------------------------------
//  | - | - | - | - | - | - | - | - |             byte              |
//  -----------------------------------------------------------------
//  byte	- if byte = '?', send name + content of all registers

//  register H
//  -----------------------------------------------------------------
//  |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
//  -----------------------------------------------------------------
//  | - |L4R|L4G|L3R|L3G|L2R|L2G|L1R|L1G|OL2|OL1|DIT| K4| K3| K2| K1|
//  -----------------------------------------------------------------
//  L4R		- LED 4 red (1 = LED ON, 0 = LED OFF)
//  L4G		- LED 4 green (1 = LED ON, 0 = LED OFF)
//  L3R		- LED 3 red (1 = LED ON, 0 = LED OFF)
//  L3G		- LED 3 green (1 = LED ON, 0 = LED OFF)
//  L2R		- LED 2 red (1 = LED ON, 0 = LED OFF)
//  L2G		- LED 2 green (1 = LED ON, 0 = LED OFF)
//  L1R		- LED 4 red (1 = LED ON, 0 = LED OFF)
//  L1G		- LED 4 green (1 = LED ON, 0 = LED OFF)
//  OL2		- backlight for output binding posts 1 (1 = LED ON, 0 = LED OFF)
//  OL1		- backlight for output binding posts 2 (1 = LED ON, 0 = LED OFF)
//  DIT		- 0 = dithering OFF, 1 = dithering ON
//  Kx		- 0 = relay in default state, 1 = relay switched

//	register I
//  ---------------------------------------------------------------------------------------------------------------------------------
//  |31 |30 |29 |28 |27 |26 |25 |24 |23 |22 |21 |20 |19 |18 |17 |16 |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
//  ---------------------------------------------------------------------------------------------------------------------------------
//  | - | - | - | - | - | - | - | - |                                 voltage                                       |   dithering   |
//  ---------------------------------------------------------------------------------------------------------------------------------
//  voltage    - 20-bit code for DC generation when dithering is OFF, highest 20 bits when dithering is ON
//  dithering  - 4 lowest bits of 24-bit code for DC generation when dithering is ON

#define L4R		14
#define L4G		13
#define L3R		12
#define L3G		11
#define L2R		10
#define L2G		9
#define L1R		8
#define L1G		7
#define OL2		6
#define OL1		5
#define DIT		4
#define K4		3
#define K3		2
#define K2		1
#define K1		0

const uint8_t module_name[5] = "@CCB";
volatile static uint16_t reg_G = 0x0000;
volatile static uint16_t reg_H = 0x0000;
volatile static uint32_t reg_I = 0x00000000;
volatile static uint8_t reg_G_update = 0;
volatile static uint8_t reg_H_update = 0;
volatile static uint8_t reg_I_update = 0;

volatile static uint8_t relays_state = 0x00;
volatile static uint8_t dithering_mode = 0;

volatile static uint32_t DAC_code = 0x00000000;
volatile static uint32_t DAC_code_dith_high = 0x00000000;
volatile static uint32_t DAC_code_dith_low = 0x00000000;
volatile static uint8_t dith_bits = 0x00;
volatile static uint16_t dith_vector = 0x0000;
volatile static uint8_t dith_counter = 0;

volatile static uint8_t byte = 0x00;

volatile static uint8_t panel_LEDs_state = 0x00;


void initPins(void);
void sortReceivedData(void);
void sendAllRegisters(void);
void updateRelays(void);
void updateLEDs(void);
void initDithTimer(void);
void ditheringON(void);
void ditheringOFF(void);
void updateDithering(void);


ISR (TIMER0_COMPA_vect)
{
	if ((dith_vector >> dith_counter) & 0x0001)
	{
		DAC_code = DAC_code_dith_high;
	}
	else
	{
		DAC_code = DAC_code_dith_low;
	}
	
	CCB_SetDACVoltage(DAC_code);
	
	if (dith_counter >= 15) {dith_counter = 0;}
	else {dith_counter++;}
}


int main(void)
{
	_delay_ms(100);
	CCB_InitRelays();
	updateRelays();							//set relays to default state (range 1, output OFF)
    UART_Init(9600, F_CPU);					//init UART for communication with controlling module
	SPI_Init(4, MSB_FIRST, SPI_MODE_1);		//init SPI for control of DAC11001B
	DDRB |= (1 << DAC_CS);					//DAC_CS as output
	DDRB |= (1 << DAC_LDAC);				//DAC_LDAC as output
	DDRB |= (1 << DAC_CLR);					//DAC_CLR as output
	CS_HIGH();
	LDAC_HIGH();
	CLR_HIGH();
	
	sei();									//enable interrupts
	
	I2C_Init(50000, F_CPU);					//init I2C
	
	_delay_ms(100);
	
	SPI_Transmit32(0);
	CCB_InitDAC();							//configure DAC registers
	//CCB_InitLEDs();
	initDithTimer();						//init timer interrupt for dithering
	
	DDRB |= (1 << DDB5);

	_delay_ms(100);
	
	CCB_RelayRESET(1);
	CCB_RelayRESET(2);
	CCB_RelayRESET(3);
	CCB_RelayRESET(4);						//output OFF
	CCB_SetDACVoltage(0x00000000);			//start with 0V -> 0A
	
	_delay_ms(100);
	
    while (1)
    {
		if (UART_AvailableBytes() > 0)
		{
			sortReceivedData();
		}
		
		//=============================================================================
		//if reg_G is "003F" ('0','?'), send content of all registers to control module
		if (reg_G_update == 1)
		{
			if (reg_G == 0x003F) {sendAllRegisters();}
				
			reg_G_update = 0;	//clear register update flag
		}
		
		//=====================================================================
		//switch relays, turn on/off LEDs and start/stop dithering if necessary
		if (reg_H_update == 1)
		{
			updateRelays();
			//updateLEDs();
			
			//turn on/off dithering
			dithering_mode = (reg_H >> DIT) & 0x0001;
			if (dithering_mode == 1) {updateDithering(); ditheringON();}
			else {ditheringOFF();}
			
			reg_H_update = 0;	//clear register update flag
		}
		
		//=================================================
		//set DAC voltage and update dithering if necessary
		if (reg_I_update == 1)
		{
			DAC_code = (reg_I >> 4) & 0x000FFFFF;
			dith_bits = reg_I & 0x0000000F;
			
			if (dithering_mode == 0)
			{
				CCB_SetDACVoltage(DAC_code);
			}
			else
			{
				updateDithering();
			}
			reg_I_update = 0;	//clear register update flag
		}
    }
}


void sortReceivedData(void)
{
	uint8_t string[35];
	uint8_t i = 0;
	
	_delay_ms(50);		//wait for reception of all data
	
	while (UART_AvailableBytes() > 0)
	{		
		i = 0;
		//read one line ending with "\n\r" or "\r\n"
		while (UART_AvailableBytes() > 0)
		{
			byte = UART_ReadByte();
			if ((byte == '\n') || (byte == '\r')) {UART_ReadByte(); break;}		//read one more byte and break
			string[i++] = byte;
		}
		string[i] = '\0';
		
		//save data into correct register
		if (string[0] == 'G')
		{
			Utils_RemoveCharFromString(string, 0);
			reg_G = Utils_HexStringToInt(string);
			reg_G_update = 1;
		}
		else if (string[0] == 'H')
		{
			Utils_RemoveCharFromString(string, 0);
			reg_H = Utils_HexStringToInt(string);
			reg_H_update = 1;
		}
		else if (string[0] == 'I')
		{
			Utils_RemoveCharFromString(string, 0);
			reg_I = Utils_HexStringToInt(string);
			reg_I_update = 1;
		}
	}
}


void sendAllRegisters(void)
{
	uint8_t string[10];
	
	UART_SendString(module_name);
	UART_SendByte('\n');
	UART_SendByte('G');
	Utils_IntToHexString(reg_G, string, 4);
	UART_SendString(string);
	UART_SendByte('\n');
	UART_SendByte('H');
	Utils_IntToHexString(reg_H, string, 4);
	UART_SendString(string);
	UART_SendByte('\n');
	UART_SendByte('I');
	Utils_IntToHexString(reg_I, string, 8);
	UART_SendString(string);
	UART_SendByte('\n');
	UART_SendByte('\r');
}


void updateRelays(void)
{
	for (uint8_t i = 0; i <= 3; i++)
	{
		if ((reg_H >> i) & 0x0001)
		{
			if (((relays_state >> i) & 0x0001) != 1) {CCB_RelaySET(i+1);}
		}
		else
		{
			if (((relays_state >> i) & 0x0001) != 0) {CCB_RelayRESET(i+1);}
		}
	}
	
	relays_state = reg_H & 0x000F;
}


void updateLEDs(void)
{
	panel_LEDs_state = reg_H >> 7;
	
	CCB_SetLEDs(panel_LEDs_state);
	
	OUT_LEDS_PORT &= ~(1 << OUT_LED_2) & ~(1 << OUT_LED_1);
	OUT_LEDS_PORT |= ((reg_H >> OL2) & 0x0001);
	OUT_LEDS_PORT |= ((reg_H >> OL1) & 0x0001);
}


void initDithTimer(void)
{
	//init Timer 0 in Clear Timer on Compare Match (CTC) Mode
	TCCR0A = 0x00;
	TCCR0A |= (1 << WGM01);					//CTC mode
	
	TCCR0B = 0x00;
	TCCR0B |= (1 <<  CS01) | (1 <<  CS01);	//set prescaler to 64
	OCR0A = 124;							//interrupt with 1kHz frequency
	
	TIMSK0 = 0x00;				//after initialization, interrupt is not enabled
}


void ditheringON(void)
{
	TIMSK0 |= (1 << OCIE0A);	//timer/counter interrupt on compare match A enable
}


void ditheringOFF(void)
{
	TIMSK0 &= ~(1 << OCIE0A);	//timer/counter interrupt on compare match A disable
}


void updateDithering(void)
{
	DAC_code = (reg_I >> 4) & 0x000FFFFF;
	
	if (DAC_code == 0x000FFFFF)
	{
		DAC_code_dith_high = DAC_code;
	}
	else
	{
		DAC_code_dith_high = DAC_code + 0x00000001;
	}
	DAC_code_dith_low = DAC_code;
	
	switch (dith_bits)
	{
		case 0:	dith_vector = 0b0000000000000000; break;
		case 1: dith_vector = 0b1000000000000000; break;
		case 2: dith_vector = 0b1000000010000000; break;
		case 3: dith_vector = 0b1000010000100000; break;
		case 4: dith_vector = 0b1000100010001000; break;
		case 5: dith_vector = 0b1001001001001000; break;
		case 6: dith_vector = 0b1010010010010010; break;
		case 7: dith_vector = 0b1010010101001010; break;
		case 8: dith_vector = 0b1010101010101010; break;
		case 9: dith_vector = 0b1110101010101010; break;
		case 10: dith_vector = 0b1101101101101010; break;
		case 11: dith_vector = 0b1101101101101110; break;
		case 12: dith_vector = 0b1110111011101110; break;
		case 13: dith_vector = 0b1111101111011110; break;
		case 14: dith_vector = 0b1111111011111110; break;
		case 15: dith_vector = 0b1111111111111110; break;
		default: dith_vector = 0b0000000000000000; break;
	}
}
