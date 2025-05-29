#include "STM32F429ZI_Delay.h"


void InitDelayTimer(void)
{
	RCC->APB1RSTR |= RCC_APB1RSTR_TIM7RST;	//reset TIM7 registers
	RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM7RST;	//clear reset of TIM7 registers
	
	RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;			//enable clock source for timer 7
	TIM7->PSC = 89;													//counter prescaler (90 - 1) from APB1 timer clock (90 MHz), counter clock frequency 1MHz = 90MHz / 90
	TIM7->ARR = 0xFFFF;											//value of auto-reload register, counter counts from 0 to ARR
	TIM7->CR1 |= TIM_CR1_CEN;								//enable counter
	while(!(TIM7->SR & TIM_SR_UIF));				//wait until update interrupt flag
}


void delay_us(uint16_t us)
{
	TIM7->CNT = 0;
	while(TIM7->CNT < us);
}


void delay_ms(uint16_t ms)
{
	for (uint16_t i = 0; i < ms; i++)
	{
		delay_us(1000);
	}
}
