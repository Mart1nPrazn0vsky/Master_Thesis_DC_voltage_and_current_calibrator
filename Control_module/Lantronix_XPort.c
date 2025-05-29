#include "Lantronix_XPort.h"


UART* UART_LANTRONIX;
GPIO_TypeDef *XPort_reset_GPIO_port;
volatile static uint8_t XPort_reset_pin;


void Lantronix_XPort_Init(UART *UART_handle, uint32_t speed, GPIO_TypeDef *reset_port, uint8_t reset_pin)
{
	UART_LANTRONIX = UART_handle;
	
	XPort_reset_GPIO_port = reset_port;
	XPort_reset_pin = reset_pin;
	
	uint8_t string[10];
	
	GPIO_InitPin(XPort_reset_GPIO_port, XPort_reset_pin, OUTPUT, PUSH_PULL, HIGH_SPEED, 0);
	
	Lantronix_XPort_Reset();
	
	delay_ms(3000);
	
	UART_SendString(UART_LANTRONIX, "xxx\r");
	
	while (UART_AvailableBytes(UART_LANTRONIX) == 0);			//wait until something is received
	delay_ms(4000);																				//wait until all data are received
	UART_ClearRXBuffer(UART_LANTRONIX);										//clear RX buffer
	
	UART_SendByte(UART_LANTRONIX, '1');
	UART_SendByte(UART_LANTRONIX, '\r');
	while (UART_AvailableBytes(UART_LANTRONIX) == 0);			//wait until something is received
	delay_ms(100);
	
	UART_ClearRXBuffer(UART_LANTRONIX);										//clear RX buffer
	
	sprintf(string, "%d\r", speed);
	UART_SendString(UART_LANTRONIX, string);							//set speed
	
	for (uint8_t i = 0; i < 18; i++)											//press Enter 17 times
	{
		while (UART_AvailableBytes(UART_LANTRONIX) == 0);		//wait until something is received
		while (UART_AvailableBytes(UART_LANTRONIX) > 0)
		{
			uint8_t byte = UART_ReadByte(UART_LANTRONIX);
		}
		UART_SendByte(UART_LANTRONIX, '\r');
		delay_ms(100);
	}
	
	while (UART_AvailableBytes(UART_LANTRONIX) == 0);		//wait until something is received
	while (UART_AvailableBytes(UART_LANTRONIX) > 0)
	{
		UART_ReadByte(UART_LANTRONIX);
	}
	
	delay_ms(100);
	UART_SendByte(UART_LANTRONIX, '9');
	UART_SendByte(UART_LANTRONIX, '\r');
	
	delay_ms(100);
	while (UART_AvailableBytes(UART_LANTRONIX) > 0) {uint8_t byte = UART_ReadByte(UART_LANTRONIX); UART_SendByte(UART_LANTRONIX, byte);}
	
	UART_ClearRXBuffer(UART_LANTRONIX);									//clear RX buffer
}


void Lantronix_XPort_Reset(void)
{
	GPIO_WritePin(XPort_reset_GPIO_port, XPort_reset_pin, LOW);
	delay_ms(500);
	GPIO_WritePin(XPort_reset_GPIO_port, XPort_reset_pin, HIGH);
}


void Lantronix_XPort_GetIPAddress(uint8_t *string_IP, uint8_t *string_gateway, uint8_t *string_mask, uint8_t *string_DNS)
{
	uint8_t string[100];
	
	//1. enter monitor mode by typing "zzz"
	//2. wait for response
	//3. type "NC" (network connection), device will respond by sending IP address, gateway, subnet mask and DNS server
	//4. read string
	//5. type "QU" to quit monitor mode
	//6. at this point, Ethernet can send/read data
	
	Lantronix_XPort_Reset();
	
	delay_ms(3000);
	UART_SendString(UART_LANTRONIX, "zzz\r");						//send "zzz" to enter monitor mode
	
	while (UART_AvailableBytes(UART_LANTRONIX) == 0);		//wait until something is received
	for (uint8_t i = 0; i < 5; i++)
	{
		UART_SendByte(UART_LANTRONIX, '\r');							//receive a few strings such as: *** NodeSet 2.0 ***
		delay_ms(100);
	}
	UART_ClearRXBuffer(UART_LANTRONIX);									//clear RX buffer
	
	UART_SendString(UART_LANTRONIX, "NC\r");						//network connection, show IP address, gateway, subnet mask, DNS server
	while (UART_AvailableBytes(UART_LANTRONIX) == 0);		//wait until something is received
	delay_ms(100);
	
	while (1)
	{
		UART_ReadLine(UART_LANTRONIX, string);
		if ((string[0] == 'I') && (string[1] == 'P')) {break;}		//read received lines until line that starts with "IP"
	}
	
	delay_ms(100);
	UART_SendString(UART_LANTRONIX, "QU\r");						//quit monitor mode
	delay_ms(100);
	
	UART_ClearRXBuffer(UART_LANTRONIX);									//clear RX buffer
	
	//received string: IP 169.254.005.151 GW 000.000.000.000 Mask 255.255.000.000 NS 000.000.000.000
	//0					10				20				30				40				50				60				70
	//01234567890123456789012345678901234567890123456789012345678901234567890123456
	//IP 169.254.005.151 GW 000.000.000.000 Mask 255.255.000.000 NS 000.000.000.000
	strncpy(string_IP, string + 3, 15);
	strncpy(string_gateway, string + 22, 15);
	strncpy(string_mask, string + 43, 15);
	strncpy(string_DNS, string + 62, 15);
}
