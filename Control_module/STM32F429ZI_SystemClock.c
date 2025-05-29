#include "STM32F429ZI_SystemClock.h"


#define PLLN		180
#define PLLP		0


void SystemClockConfig(uint32_t HSE_freq, uint8_t HSE_source)
{
	//8MHz clock signal is at the input of OSC_IN pin on Nucleo board, configure HSE (high-speed external clock signal)
	RCC->CR |= (HSE_source << RCC_CR_HSEBYP_Pos);			//write 0 or 1 to HSEBYP (bit 18)
	RCC->CR |= RCC_CR_HSEON;													//HSE clock enable
	while(!(RCC->CR & RCC_CR_HSERDY));								//wait until HSE clock is ready
	
	//max. 180MHz HCLK can be achieved on STM32F42xxx by turning on over-drive mode
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;			//power interface clock enable
	PWR->CR |= PWR_CR_VOS_0 | PWR_CR_VOS_1;	//VOS bits to 0b11 (regulator voltage scale 1)
	PWR->CR |= PWR_CR_ODEN;									//enable over-drive mode
	while(!(PWR->CSR & PWR_CSR_ODRDY));			//wait until over-drive mode is ready
	PWR->CR |= PWR_CR_ODSWEN;								//switch voltage regulator to over-drive mode
	while(!(PWR->CSR & PWR_CSR_ODSWRDY));		//wait until over-drive mode is active
	
	//settings for effective and faster processor funtionality
	FLASH->ACR |= FLASH_ACR_ICEN;						//enable instruction cache
	FLASH->ACR |= FLASH_ACR_DCEN;						//enable data cache
	FLASH->ACR |= FLASH_ACR_PRFTEN;					//enable prefetch
	FLASH->ACR |= FLASH_ACR_LATENCY_5WS;		//latency - 5 wait states (voltage range 2.7V - 3.6V, 150MHz < HCLK <= 180MHz)
	
	//set prescalers for timers, peripherals etc.
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;				//set AHB prescaler to 1 (system clock not divided)
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;				//set APB1 (low-speed prescaler) to 4 (to get 45MHz from main 180MHz)
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;				//set APB2 (high-speed prescaler) to 2 (to get 90MHz from main 180MHz)
	
	//configure main PLL
	RCC->PLLCFGR = 0x24000000;															//clear bits PLLP, PLLN, PLLM
	uint32_t PLLM = (PLLN * HSE_freq) / (2 * 180000000);		//calculate value of PLLM
	RCC->PLLCFGR |= (PLLM << 0);														//division factor for main PLL
	RCC->PLLCFGR |= (PLLN << 6);														//main PLL multiplication factor
	RCC->PLLCFGR |= (PLLP << 16);														//main PLL division factor for main system clock
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC;											//HSE oscillator clock as PLL entry
	
	RCC->CR |= RCC_CR_PLLON;								//PLL on
	while(!(RCC->CR & RCC_CR_PLLRDY));			//wait until PLL is locked
	
	RCC->CFGR |= RCC_CFGR_SW_PLL;														//select PLL as system clock
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);	//wait until PLL is switched as system clock
}
