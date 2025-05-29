#include "STM32F429ZI_UART.h"


volatile static uint8_t UART1_RX_buffer[UART1_RX_BUFFER_SIZE];
volatile static uint8_t UART1_RX_counter = 0;
volatile static uint8_t UART1_RX_write_pos = 0;
volatile static uint8_t UART1_RX_read_pos = 0;

volatile static uint8_t UART2_RX_buffer[UART2_RX_BUFFER_SIZE];
volatile static uint8_t UART2_RX_counter = 0;
volatile static uint8_t UART2_RX_write_pos = 0;
volatile static uint8_t UART2_RX_read_pos = 0;

volatile static uint8_t UART3_RX_buffer[UART3_RX_BUFFER_SIZE];
volatile static uint8_t UART3_RX_counter = 0;
volatile static uint8_t UART3_RX_write_pos = 0;
volatile static uint8_t UART3_RX_read_pos = 0;

volatile static uint8_t UART4_RX_buffer[UART4_RX_BUFFER_SIZE];
volatile static uint8_t UART4_RX_counter = 0;
volatile static uint8_t UART4_RX_write_pos = 0;
volatile static uint8_t UART4_RX_read_pos = 0;

volatile static uint8_t UART5_RX_buffer[UART5_RX_BUFFER_SIZE];
volatile static uint8_t UART5_RX_counter = 0;
volatile static uint8_t UART5_RX_write_pos = 0;
volatile static uint8_t UART5_RX_read_pos = 0;

volatile static uint8_t UART6_RX_buffer[UART6_RX_BUFFER_SIZE];
volatile static uint8_t UART6_RX_counter = 0;
volatile static uint8_t UART6_RX_write_pos = 0;
volatile static uint8_t UART6_RX_read_pos = 0;

volatile static uint8_t UART7_RX_buffer[UART7_RX_BUFFER_SIZE];
volatile static uint8_t UART7_RX_counter = 0;
volatile static uint8_t UART7_RX_write_pos = 0;
volatile static uint8_t UART7_RX_read_pos = 0;

volatile static uint8_t UART8_RX_buffer[UART8_RX_BUFFER_SIZE];
volatile static uint8_t UART8_RX_counter = 0;
volatile static uint8_t UART8_RX_write_pos = 0;
volatile static uint8_t UART8_RX_read_pos = 0;


UART UART1_handle;
UART UART2_handle;
UART UART3_handle;
UART UART4_handle;
UART UART5_handle;
UART UART6_handle;
UART UART7_handle;
UART UART8_handle;


void USART1_IRQHandler(void)
{
	if (USART1->SR & (USART_SR_RXNE))
	{
		UART1_RX_buffer[UART1_RX_write_pos++] = USART1->DR;		//increment write position
		UART1_RX_counter++;																		//increment counter of bytes in buffer
		
		if (UART1_RX_write_pos >= UART1_RX_BUFFER_SIZE)		{UART1_RX_write_pos = 0;}
	}
}


void USART2_IRQHandler(void)
{
	if (USART2->SR & (USART_SR_RXNE))
	{
		UART2_RX_buffer[UART2_RX_write_pos++] = USART2->DR;		//increment write position
		UART2_RX_counter++;																		//increment counter of bytes in buffer
		
		if (UART2_RX_write_pos >= UART2_RX_BUFFER_SIZE)		{UART2_RX_write_pos = 0;}
	}
}


void USART3_IRQHandler(void)
{
	if (USART3->SR & (USART_SR_RXNE))
	{
		UART3_RX_buffer[UART3_RX_write_pos++] = USART3->DR;		//increment write position
		UART3_RX_counter++;																		//increment counter of bytes in buffer
		
		if (UART3_RX_write_pos >= UART3_RX_BUFFER_SIZE)		{UART3_RX_write_pos = 0;}
	}
}


void UART4_IRQHandler(void)
{
	if (UART4->SR & (USART_SR_RXNE))
	{
		UART4_RX_buffer[UART4_RX_write_pos++] = UART4->DR;		//increment write position
		UART4_RX_counter++;																		//increment counter of bytes in buffer
		
		if (UART4_RX_write_pos >= UART4_RX_BUFFER_SIZE)		{UART4_RX_write_pos = 0;}
	}
}



void UART5_IRQHandler(void)
{
	if (UART5->SR & (USART_SR_RXNE))
	{
		UART5_RX_buffer[UART5_RX_write_pos++] = UART5->DR;		//increment write position
		UART5_RX_counter++;																		//increment counter of bytes in buffer
		
		if (UART5_RX_write_pos >= UART5_RX_BUFFER_SIZE)		{UART5_RX_write_pos = 0;}
	}
}


void USART6_IRQHandler(void)
{	
	if (USART6->SR & (USART_SR_RXNE))
	{
		UART6_RX_buffer[UART6_RX_write_pos++] = USART6->DR;		//increment write position
		UART6_RX_counter++;																		//increment counter of bytes in buffer
		
		if (UART6_RX_write_pos >= UART6_RX_BUFFER_SIZE)		{UART6_RX_write_pos = 0;}
	}
}


void UART7_IRQHandler(void)
{
	if (UART7->SR & (USART_SR_RXNE))
	{
		UART7_RX_buffer[UART7_RX_write_pos++] = UART7->DR;		//increment write position
		UART7_RX_counter++;																		//increment counter of bytes in buffer
		
		if (UART7_RX_write_pos >= UART7_RX_BUFFER_SIZE)		{UART7_RX_write_pos = 0;}
	}
}


void UART8_IRQHandler(void)
{
	if (UART8->SR & (USART_SR_RXNE))
	{
		UART8_RX_buffer[UART8_RX_write_pos++] = UART8->DR;		//increment write position
		UART8_RX_counter++;																		//increment counter of bytes in buffer
		
		if (UART8_RX_write_pos >= UART8_RX_BUFFER_SIZE)		{UART8_RX_write_pos = 0;}
	}
}


UART *UART_Init(USART_TypeDef *UARTx, uint32_t baud_rate, uint32_t CLK_FREQ, uint8_t priority, GPIO_TypeDef *TX_port, uint8_t TX_pin, GPIO_TypeDef *RX_port, uint8_t RX_pin)
{
	uint8_t alternate_function = 0;
	if ((UARTx == USART1) || (UARTx == USART2) || (UARTx == USART3)) {alternate_function = 7;}
	else {alternate_function = 8;}
	
	GPIO_InitPin(TX_port, TX_pin, ALTERNATE_FUNCTION, 0, VERY_HIGH_SPEED, 0);		//init TX pin
	GPIO_AlternateFunction(TX_port, TX_pin, alternate_function);
	GPIO_InitPin(RX_port, RX_pin, ALTERNATE_FUNCTION, 0, VERY_HIGH_SPEED, 0);		//init RX pin
	GPIO_AlternateFunction(RX_port, RX_pin, alternate_function);	
	
	if (UARTx == USART1)
	{
		UART1_handle.UARTx = USART1;
		UART1_handle.UART_RX_buffer = UART1_RX_buffer;
		UART1_handle.UART_RX_counter = &UART1_RX_counter;
		UART1_handle.UART_RX_read_pos = &UART1_RX_read_pos;
		UART1_handle.UART_RX_write_pos = &UART1_RX_write_pos;
		UART1_handle.UART_RX_buffer_size = UART1_RX_BUFFER_SIZE;
		
		RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;	//reset USART1 registers
		RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;	//clear reset of USART1 registers
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;			//enable USART1 clock
		NVIC_SetPriority(USART1_IRQn, priority);	//set USART1 interrupt priority
		NVIC_EnableIRQ(USART1_IRQn);							//enable USART1 interrupt
	}
	else if (UARTx == USART2)
	{
		UART2_handle.UARTx = USART2;
		UART2_handle.UART_RX_buffer = UART2_RX_buffer;
		UART2_handle.UART_RX_counter = &UART2_RX_counter;
		UART2_handle.UART_RX_read_pos = &UART2_RX_read_pos;
		UART2_handle.UART_RX_write_pos = &UART2_RX_write_pos;
		UART2_handle.UART_RX_buffer_size = UART2_RX_BUFFER_SIZE;
		
		RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;	//reset USART2 registers
		RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;	//clear reset of USART2 registers
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;			//enable USART2 clock
		NVIC_SetPriority(USART2_IRQn, priority);	//set USART2 interrupt priority
		NVIC_EnableIRQ(USART2_IRQn);							//enable USART2 interrupt
	}
	else if (UARTx == USART3)
	{
		UART3_handle.UARTx = USART3;
		UART3_handle.UART_RX_buffer = UART3_RX_buffer;
		UART3_handle.UART_RX_counter = &UART3_RX_counter;
		UART3_handle.UART_RX_read_pos = &UART3_RX_read_pos;
		UART3_handle.UART_RX_write_pos = &UART3_RX_write_pos;
		UART3_handle.UART_RX_buffer_size = UART3_RX_BUFFER_SIZE;
		
		RCC->APB1RSTR |= RCC_APB1RSTR_USART3RST;	//reset USART3 registers
		RCC->APB1RSTR &= ~RCC_APB1RSTR_USART3RST;	//clear reset of USART3 registers
		RCC->APB1ENR |= RCC_APB1ENR_USART3EN;			//enable USART3 clock
		NVIC_SetPriority(USART3_IRQn, priority);	//set USART3 interrupt priority
		NVIC_EnableIRQ(USART3_IRQn);							//enable USART3 interrupt
	}
	else if (UARTx == UART4)
	{
		UART4_handle.UARTx = UART4;
		UART4_handle.UART_RX_buffer = UART4_RX_buffer;
		UART4_handle.UART_RX_counter = &UART4_RX_counter;
		UART4_handle.UART_RX_read_pos = &UART4_RX_read_pos;
		UART4_handle.UART_RX_write_pos = &UART4_RX_write_pos;
		UART4_handle.UART_RX_buffer_size = UART4_RX_BUFFER_SIZE;
		
		RCC->APB1RSTR |= RCC_APB1RSTR_UART4RST;		//reset UART4 registers
		RCC->APB1RSTR &= ~RCC_APB1RSTR_UART4RST;	//clear reset of UART4 registers
		RCC->APB1ENR |= RCC_APB1ENR_UART4EN;			//enable UART4 clock
		NVIC_SetPriority(UART4_IRQn, priority);	//set USART2 interrupt priority
		NVIC_EnableIRQ(UART4_IRQn);							//enable USART2 interrupt
	}
	else if (UARTx == UART5)
	{
		UART5_handle.UARTx = UART5;
		UART5_handle.UART_RX_buffer = UART5_RX_buffer;
		UART5_handle.UART_RX_counter = &UART5_RX_counter;
		UART5_handle.UART_RX_read_pos = &UART5_RX_read_pos;
		UART5_handle.UART_RX_write_pos = &UART5_RX_write_pos;
		UART5_handle.UART_RX_buffer_size = UART5_RX_BUFFER_SIZE;
		
		RCC->APB1RSTR |= RCC_APB1RSTR_UART5RST;		//reset UART5 registers
		RCC->APB1RSTR &= ~RCC_APB1RSTR_UART5RST;	//clear reset of USART5 registers
		RCC->APB1ENR |= RCC_APB1ENR_UART5EN;			//enable UART5 clock
		NVIC_SetPriority(UART5_IRQn, priority);		//set UART5 interrupt priority
		NVIC_EnableIRQ(UART5_IRQn);								//enable UART5 interrupt
	}
	else if (UARTx == USART6)
	{
		UART6_handle.UARTx = USART6;
		UART6_handle.UART_RX_buffer = UART6_RX_buffer;
		UART6_handle.UART_RX_counter = &UART6_RX_counter;
		UART6_handle.UART_RX_read_pos = &UART6_RX_read_pos;
		UART6_handle.UART_RX_write_pos = &UART6_RX_write_pos;
		UART6_handle.UART_RX_buffer_size = UART6_RX_BUFFER_SIZE;
		
		RCC->APB2RSTR |= RCC_APB2RSTR_USART6RST;	//reset USART6 registers
		RCC->APB2RSTR &= ~RCC_APB2RSTR_USART6RST;	//clear reset of USART6 registers
		RCC->APB2ENR |= RCC_APB2ENR_USART6EN;			//enable USART6 clock
		NVIC_SetPriority(USART6_IRQn, priority);	//set USART6 interrupt priority
		NVIC_EnableIRQ(USART6_IRQn);							//enable USART6 interrupt
	}
	else if (UARTx == UART7)
	{
		UART7_handle.UARTx = UART7;
		UART7_handle.UART_RX_buffer = UART7_RX_buffer;
		UART7_handle.UART_RX_counter = &UART7_RX_counter;
		UART7_handle.UART_RX_read_pos = &UART7_RX_read_pos;
		UART7_handle.UART_RX_write_pos = &UART7_RX_write_pos;
		UART7_handle.UART_RX_buffer_size = UART7_RX_BUFFER_SIZE;
		
		RCC->APB1RSTR |= RCC_APB1RSTR_UART7RST;		//reset UART7 registers
		RCC->APB1RSTR &= ~RCC_APB1RSTR_UART7RST;	//clear reset of USART7 registers
		RCC->APB1ENR |= RCC_APB1ENR_UART7EN;			//enable UART7 clock
		NVIC_SetPriority(UART7_IRQn, priority);		//set UART7 interrupt priority
		NVIC_EnableIRQ(UART7_IRQn);								//enable UART7 interrupt
	}
	else if (UARTx == UART8)
	{
		UART8_handle.UARTx = UART8;
		UART8_handle.UART_RX_buffer = UART8_RX_buffer;
		UART8_handle.UART_RX_counter = &UART8_RX_counter;
		UART8_handle.UART_RX_read_pos = &UART8_RX_read_pos;
		UART8_handle.UART_RX_write_pos = &UART8_RX_write_pos;
		UART8_handle.UART_RX_buffer_size = UART8_RX_BUFFER_SIZE;
		
		RCC->APB1RSTR |= RCC_APB1RSTR_UART7RST;		//reset UART7 registers
		RCC->APB1RSTR &= ~RCC_APB1RSTR_UART7RST;	//clear reset of USART7 registers
		RCC->APB1ENR |= RCC_APB1ENR_UART7EN;			//enable UART7 clock
		NVIC_SetPriority(UART7_IRQn, priority);		//set UART7 interrupt priority
		NVIC_EnableIRQ(UART7_IRQn);								//enable UART7 interrupt
	}
	
	float USARTDIV = (float) CLK_FREQ / (16 * baud_rate);					//calculation of usart divider (formula in datasheet)
	uint32_t USARTDIV_int = (uint32_t) USARTDIV;									//get integer part of usart divider
	float USARTDIV_fraction = (float) USARTDIV - USARTDIV_int;		//get fraction of usart divider
	float USARTDIV_fraction16 = (float) USARTDIV_fraction * 16;		//multiply usart divider fraction by 16
	
	if (USARTDIV_fraction16 >= 15.5)					//if fraction*16 is rounded to 16, overflow on fraction
	{
		USARTDIV_int = USARTDIV_int + 1;				//increment mantissa
		UARTx->BRR = (USARTDIV_int << 4);				//save mantissa into register
	}
	else																			//if fraction*16 is rounded to 15 or less, convert fraction*16 to integer
	{
		uint8_t USARTDIV_fraction16_int = (uint8_t) USARTDIV_fraction16;			//convert fraction*16 to integer
		if (((float) USARTDIV_fraction16 - USARTDIV_fraction16_int) >= 0.5)		//if fraction*16 is rounded to higher number, increment it
		{
			USARTDIV_fraction16_int = USARTDIV_fraction16_int + 1;							//increment fraction*16
		}
		
		UARTx->BRR = (USARTDIV_int << 4);								//save mantissa into register
		UARTx->BRR |= (0x0F & USARTDIV_fraction16_int);	//save fraction*16 into register
	}
	
	UARTx->CR1 |= USART_CR1_UE;							//USART3 enable
	UARTx->CR1 |= USART_CR1_TE;							//enable transmitter
	UARTx->CR1 |= USART_CR1_RE;							//enable receiver
	UARTx->CR1 |= USART_CR1_RXNEIE;					//enable RX interrupt
	
	if (UARTx == USART1) {return &UART1_handle;}
	else if (UARTx == USART2) {return &UART2_handle;}
	else if (UARTx == USART3) {return &UART3_handle;}
	else if (UARTx == UART4) {return &UART4_handle;}
	else if (UARTx == UART5) {return &UART5_handle;}
	else if (UARTx == USART6) {return &UART6_handle;}
	else if (UARTx == UART7) {return &UART7_handle;}
	else {return &UART8_handle;}
}


void UART_SendByte(UART *UARTx, uint8_t byte)
{
	while (!((UARTx->UARTx)->SR & (USART_SR_TXE)));		//wait until data register is empty and ready for next transmission
	(UARTx->UARTx)->DR = byte;												//put byte into data register
}


void UART_SendString(UART *UARTx, uint8_t *string)
{
	uint8_t i = 0;
	
	while(string[i] != '\0')
	{
		UART_SendByte(UARTx, string[i++]);
	}		
}


uint8_t UART_ReadByte(UART *UARTx)
{
	uint8_t data = 0;
	
	data = UARTx->UART_RX_buffer[*(UARTx->UART_RX_read_pos)];		//increment read position
	*(UARTx->UART_RX_read_pos) += 1;
	*(UARTx->UART_RX_counter) -= 1;															//decrement counter of bytes in buffer
	
	if (*(UARTx->UART_RX_read_pos) >= UARTx->UART_RX_buffer_size)		{*(UARTx->UART_RX_read_pos) = 0;}
	
	return data;
}


void UART_ReadLine(UART *UARTx, uint8_t *string)
{
	uint8_t byte;
	uint8_t i = 0;
	
	do
	{
		while (UART_AvailableBytes(UARTx) == 0);	//wait until something is available in RX buffer
		byte = UART_ReadByte(UARTx);
		string[i++] = byte;
	} while ((byte != '\n') && (byte != '\r'));
	
	string[i-1] = '\0';		//i-1 to remove '\n' from string
}


uint8_t UART_AvailableBytes(UART *UARTx)
{
	return *(UARTx->UART_RX_counter);
}


void UART_ClearRXBuffer(UART *UARTx)
{
	for (uint8_t i = 0; i < UARTx->UART_RX_buffer_size; i++)
	{
		UARTx->UART_RX_buffer[i] = 0;			//set content of each byte in RX buffer to 0
	}
	
	*(UARTx->UART_RX_counter) = 0;
	*(UARTx->UART_RX_write_pos) = 0;
	*(UARTx->UART_RX_read_pos) = 0;
}
