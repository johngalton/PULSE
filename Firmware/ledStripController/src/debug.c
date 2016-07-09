/*
 * debug.c
 *
 *  Created on: 2 Jun 2016
 *      Author: John
 */

#include "debug.h"

USART_HandleTypeDef USART_Handle;
DMA_HandleTypeDef 	DMA_TX_Handle;
DMA_HandleTypeDef 	DMA_RX_Handle;

uint8_t rx_buffer[100] = {0};
uint8_t tx_buffer[100] = {0};

void debug_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOB_CLK_ENABLE();

	//Initialise B0 and B1
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
/*
	//Debug TX
	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//Debug RX
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

	//Initialisation
	USART_Handle.Init.BaudRate = 9600;
	USART_Handle.Init.Parity = USART_PARITY_NONE;
	USART_Handle.Init.StopBits = USART_STOPBITS_1;
	USART_Handle.Init.WordLength = USART_WORDLENGTH_8B;
	USART_Handle.Init.Mode = USART_MODE_TX_RX;
	USART_Handle.Init.CLKPolarity = USART_POLARITY_LOW;
	USART_Handle.Init.CLKPhase = USART_PHASE_1EDGE;
	USART_Handle.Init.CLKLastBit = USART_LASTBIT_DISABLE;
	//Instance
	USART_Handle.Instance = USART2;

	//Initialisation
	DMA_TX_Handle.Init.Request = DMA_REQUEST_4;
	DMA_TX_Handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
	DMA_TX_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
	DMA_TX_Handle.Init.MemInc = DMA_MINC_ENABLE;
	DMA_TX_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	DMA_TX_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	DMA_TX_Handle.Init.Mode = DMA_NORMAL;
	DMA_TX_Handle.Init.Priority = DMA_PRIORITY_MEDIUM;
	//Instance
	DMA_TX_Handle.Instance = DMA1_Channel4;

	DMA_RX_Handle.Init = DMA_TX_Handle.Init;
	DMA_RX_Handle.Instance = DMA1_Channel5;


	HAL_USART_Init(&USART_Handle);
	HAL_DMA_Init(&DMA_TX_Handle);
	HAL_DMA_Init(&DMA_RX_Handle);
*/

	/*Set it to on by default */
	DEBUG_G_LED_ON;
	DEBUG_Y_LED_OFF;
}

void debug_disable_usart(void)
{

}

void debug_enable_usart(void)
{

}

