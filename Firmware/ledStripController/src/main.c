/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "stm32l0xx_it.h"

#include "debug.h"
#include "led_driver.h"
#include "timer.h"
#include "uart.h"
#include <math.h>

void initialise(void);
void shortHandler(uint8_t *data);
void longHandler(uart_long_function data);
void timHandler(void);

uint8_t id = DEV_ID;
volatile uint8_t led_updated = 0;

int main(void)
{
	float value = 0.2;

	value = log(value);

	initialise();

	while (1)
	{
		if (led_updated == 1)
		{
			led_propagate();
			led_updated = 0;
		}
		else
		{
			uart_handle();
		}
	}
}

void initialise(void)
{
	uart_init(shortHandler, longHandler);
	debug_init();
	timer_init(timHandler);
	led_init();
}

void clock_init(void)
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

	HAL_NVIC_SetPriority(SysTick_IRQn, 1, 0);
}

void shortHandler(uint8_t *data)
{
	switch (data[0])
	{
		case 0:
		{
			uint8_t colour = data[1 + (2 * (id - 1))];
			uint8_t length = data[2 + (2 * (id - 1))];
			for (uint8_t count = 0; count < length; count++)
			{
				led_push_buffer(colour);
			}
		}
		break;
		case 1:
		{
			uint8_t colour = data[1 + (2 * (id - 1))];
			led_set_beacon(colour);
		}
		break;
		case 2:
		{
			uint8_t colour = data[1 + (2 * (id - 1))];
			led_set_all(colour);
		}
		break;
		case 3:
		{
			uint8_t colour = data[1 + (2 * (id - 1))];
			led_set_beacon_background(colour);
		}
		break;
		case 4:
		{
			uint8_t col = data[1 + (2 * (id - 1))];
			uint8_t value = data[2 + (2 * (id - 1))];

			if (value > 0)
				led_pulse_target(col);
		}
		break;
		case 5:
		{
			uint8_t col = data[1+(2*(id-1))];
			uint8_t offset = data[2+(2*(id-1))];

			//Greatest allowed offset
			if ((offset+29) > STRIP_SIZE)
				offset = STRIP_SIZE - 29;

			led_show_logo(col, offset);
		}
		break;
		default:
		return;
	}
}

void longHandler(uart_long_function data)
{
	if (data.address != id && data.address != 0)
		return;

	switch (data.command)
	{
		case 1:
		{
			if (data.length != 2)
				return;

			uint16_t value = data.data[0] << 8 | data.data[1];
			timer_set_interval(value);
		}
		break;
		case 2:
		{
			if (data.address == 0)
				return;

			uint16_t value = timer_get_interval();
			uart_send((uint8_t *)&value,2);
		}
		break;
		case 3:
		{
			if (data.length != 1)
				return;

			uint8_t value = data.data[0];
			led_set_beacon_fade_div(value);
		}
		break;
		case 4:
		{
			//Set the whole bar to 0
			led_show_bar(0,0,STRIP_SIZE);
		}
		break;
		case 8:
		{
			if (data.length != 1)
				return;

			//Get the value we're setting
			uint8_t colour = data.data[0];
			uint8_t offset = data.data[1];
			uint8_t length = data.data[2];

			led_show_bar(colour, offset, length);
		}
		break;
	}
}

void timHandler(void)
{
	led_update();
	led_updated = 1;
}
