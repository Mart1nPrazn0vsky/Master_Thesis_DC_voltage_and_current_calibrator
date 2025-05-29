#include "STM32F429ZI_GPIOPins.h"


void GPIO_InitPin(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t mode, uint8_t type, uint8_t speed, uint8_t pull)
{
	if (GPIOx == GPIOA) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;}					//IO port A clock enable
	else if (GPIOx == GPIOB) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;}			//IO port B clock enable
	else if (GPIOx == GPIOC) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;}			//IO port C clock enable
	else if (GPIOx == GPIOD) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;}			//IO port D clock enable
	else if (GPIOx == GPIOE) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;}			//IO port E clock enable
	else if (GPIOx == GPIOF) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;}			//IO port F clock enable
	else if (GPIOx == GPIOG) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;}			//IO port G clock enable
	else if (GPIOx == GPIOH) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;}			//IO port H clock enable
	else if (GPIOx == GPIOI) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;}			//IO port I clock enable
	else if (GPIOx == GPIOJ) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOJEN;}			//IO port J clock enable
	else if (GPIOx == GPIOK) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOKEN;}			//IO port K clock enable
		
	GPIOx->MODER &= ~(0x03 << (pin << 1));				//clear MODER bits for corresponding pin
	GPIOx->MODER |= (mode << (pin << 1));					//select I/O direction mode
	GPIOx->OTYPER &= ~(1 << pin);									//clear OTYPER bit for corresponding pin
	GPIOx->OTYPER |= (type << pin);								//select type of output pin
	GPIOx->OSPEEDR &= ~(0x03 << (pin << 1));			//clear OSPEEDR bits for corresponding pin
	GPIOx->OSPEEDR |= (speed << (pin << 1));			//select pin output speed
	GPIOx->PUPDR &= ~(0x03 << (pin << 1));				//clear PUPDR bits for corresponding pin
	GPIOx->PUPDR |= (pull << (pin << 1));					//select pull up, pull down or no pull up, no pull down for input pin
}


void GPIO_WritePin(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t value)
{	
	GPIOx->BSRR |= (1 << (pin + ((~value & 0x01) << 4)));		//if value == 0, write 1 to (pin + 16), if value == 1, write 1 to pin
}


uint8_t GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint8_t pin)
{
	return (GPIOx->IDR >> pin) & 0x0001;			//get corresponding bit from IDR register
}


void GPIO_AlternateFunction(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t function)
{
	if (pin <= 7)
	{
		GPIOx->AFR[0] &= ~(0x0F << (pin << 2));				//clear 4 bits of corresponding pin
		GPIOx->AFR[0] |= (function << (pin << 2));		//set 4 bits of corresponfing pin
	}
	else
	{
		pin -= 8;
		GPIOx->AFR[1] &= ~(0x0F << (pin << 2));				//clear 4 bits of corresponding pin
		GPIOx->AFR[1] |= (function << (pin << 2));		//set 4 bits of corresponfing pin
	}
}


void GPIO_ConfigureExternalInterrupt(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t pull, uint8_t edge, uint8_t priority)
{
	uint8_t EXTI_config;
	
	if (GPIOx == GPIOA) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; EXTI_config = 0;}					//IO port A clock enable
	else if (GPIOx == GPIOB) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; EXTI_config = 1;}		//IO port B clock enable
	else if (GPIOx == GPIOC) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; EXTI_config = 2;}		//IO port C clock enable
	else if (GPIOx == GPIOD) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; EXTI_config = 3;}		//IO port D clock enable
	else if (GPIOx == GPIOE) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; EXTI_config = 4;}		//IO port E clock enable
	else if (GPIOx == GPIOF) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN; EXTI_config = 5;}		//IO port F clock enable
	else if (GPIOx == GPIOG) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN; EXTI_config = 6;}		//IO port G clock enable
	else if (GPIOx == GPIOH) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN; EXTI_config = 7;}		//IO port H clock enable
	else if (GPIOx == GPIOI) {RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN; EXTI_config = 8;}		//IO port I clock enable
	
	GPIOx->MODER &= ~(0x03 << (pin << 1));				//clear MODER bits for corresponding pin
	GPIOx->MODER |= (INPUT << (pin << 1));				//select I/O direction mode
	GPIOx->PUPDR &= ~(0x03 << (pin << 1));				//clear PUPDR bits for corresponding pin
	GPIOx->PUPDR |= (pull << (pin << 1));					//select pull up, pull down or no pull up, no pull down for input pin
	
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;					//system configuration clock enable
	
	//configure external interrupt configuration register (select GPIO port and pin)
	if (pin <= 3)
	{
		SYSCFG->EXTICR[0] &= ~(0x0F << (pin << 2));
		SYSCFG->EXTICR[0] |= (EXTI_config << (pin << 2));
	}
	else if (pin <= 7)
	{
		SYSCFG->EXTICR[1] &= ~(0x0F << ((pin - 4) << 2));
		SYSCFG->EXTICR[1] |= (EXTI_config << ((pin - 4) << 2));
	}
	else if (pin <= 11)
	{
		SYSCFG->EXTICR[2] &= ~(0x0F << ((pin - 8) << 2));
		SYSCFG->EXTICR[2] |= (EXTI_config << ((pin - 8) << 2));
	}
	else if (pin <= 15)
	{
		SYSCFG->EXTICR[3] &= ~(0x0F << ((pin - 12) << 2));
		SYSCFG->EXTICR[3] |= (EXTI_config << ((pin - 12) << 2));
	}
	
	EXTI->IMR |= (1 << pin);						//remove interrupt request mask from corresponding bit
	
	if (edge == RISING_EDGE)
	{
		EXTI->RTSR |= (1 << pin);					//enable rising trigger
		EXTI->FTSR &= ~(1 << pin);				//disable falling trigger
	}
	else if (edge == FALLING_EDGE)
	{
		EXTI->RTSR &= ~(1 << pin);				//disable rising trigger
		EXTI->FTSR |= (1 << pin);					//enable falling trigger
	}
	else if (edge == BOTH_EDGES)
	{
		EXTI->RTSR |= (1 << pin);					//enable rising trigger
		EXTI->FTSR |= (1 << pin);					//enable falling trigger
	}
	
	//set priority and enable interrupt
	if (pin == 0) {NVIC_SetPriority(EXTI0_IRQn, priority); NVIC_EnableIRQ(EXTI0_IRQn);}
	else if (pin == 1) {NVIC_SetPriority(EXTI1_IRQn, priority); NVIC_EnableIRQ(EXTI1_IRQn);}
	else if (pin == 2) {NVIC_SetPriority(EXTI2_IRQn, priority); NVIC_EnableIRQ(EXTI2_IRQn);}
	else if (pin == 3) {NVIC_SetPriority(EXTI3_IRQn, priority); NVIC_EnableIRQ(EXTI3_IRQn);}
	else if (pin == 4) {NVIC_SetPriority(EXTI4_IRQn, priority); NVIC_EnableIRQ(EXTI4_IRQn);}
	else if (pin <= 9) {NVIC_SetPriority(EXTI9_5_IRQn, priority); NVIC_EnableIRQ(EXTI9_5_IRQn);}
	else if (pin <= 15) {NVIC_SetPriority(EXTI15_10_IRQn, priority); NVIC_EnableIRQ(EXTI15_10_IRQn);}
}
