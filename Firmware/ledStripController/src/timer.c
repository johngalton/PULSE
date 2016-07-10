/*
 * timer.c
 *
 *  Created on: 5 Jul 2016
 *      Author: ja9g1
 */

#include "timer.h"

#define CLK_FREQ	32000000

void sysClock_init(void);

volatile uint32_t sysTime = 0;

TIM_HandleTypeDef timHandle;
volatile uint8_t update_led = 0;
timerState current_state = timer_disabled;
void (*timerHandler)(void);

void timer_init(void (*handler)(void))
{
	//First initialise the system clock
	sysClock_init();

	timerHandler = handler;
	//Try and set this up with direct register access.. The new drivers are just too horible.
	//We ideally want to divide the clock down to 1Hz however this will not be possible from 32MHz
	//This is because the maximum divide is 65536
	//Instead lets divide the clock down to 1000Hz (1ms)

	RCC->APB1ENR |= 1;

	//Enable Update interrupt
	TIM2->DIER |= (1<<0);
	//Divide the clock down to 1ms
	TIM2->PSC = 32000;
	//Up counter so reset to 0
	TIM2->ARR = 0x16;
	//			ARR		URS			EN
	TIM2->CR1 |= (1<<7) | (1<<2);// | (1<<0);

	current_state = timer_enabled;

	//Enable the interrupts
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_SetPriority(TIM2_IRQn, 0);

	TIM2->CR1 |= 1;
}

void timer_set_interval(uint16_t interval)
{
	TIM2->ARR = interval;
}

uint16_t timer_get_interval(void)
{
	uint16_t value = TIM2->ARR;
	return value;
}

void timer_set_state(timerState newState)
{
	if (newState != current_state)
		return;

	//If the timer is changing state we should reset the counter
	TIM2->CNT = 0;

	if (newState == timer_enabled)
		TIM2->CR1 |= 0x01;
	else
		TIM2->CR1 &= ~(0x01);
}

timerState timer_get_state(void)
{
	return current_state;
}

void sysClock_init(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	__HAL_FLASH_PREREAD_BUFFER_ENABLE();

	__HAL_RCC_PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 16;
	RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
	RCC_OscInitStruct.MSIState = RCC_MSI_OFF;
	RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_OFF;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
	RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	HAL_RCC_OscConfig(&RCC_OscInitStruct);
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void timer_delay_ms(uint32_t delay)
{
	uint32_t endTime = sysTime + delay;

	while (sysTime < endTime) ;
}

void SysTick_Handler(void)
{
	sysTime++;
}

void TIM2_IRQHandler(void)
{
	if (TIM2->SR & 1)
	{
		TIM2->SR &= ~(1);
		timerHandler();
	}
}
