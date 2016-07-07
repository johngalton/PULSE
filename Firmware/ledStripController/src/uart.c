/*
 * uart.c
 *
 *  Created on: 7 Jul 2016
 *      Author: ja9g1
 */

#include "uart.h"

void uart_init(void)
{
	//Enable clock for USART1
	RCC->APB2ENR |= (1 << 14);

	//We are using PA9 (USART1 TX) and PA10 (USART1 RX)
	//First we should initialise the GPIO
	//Set AF mode for pins 9 and 10
	GPIOA->MODER |= ((0x02 << (2*9)) | (0x02 << (2*10)));
	GPIOA->MODER &= ~(0x01 << (2*9) | (0x01 << (2*10)));

	//Setup the alternate function registers (AF4)
	GPIOA->AFR[1] |= ((0x04 << (4 * (9 - 8))) | (0x04 << (4 * (10 - 8))));
	GPIOA->AFR[1] &= ~((0x0B << (4 * (9 - 8))) | (0x0B << (4 * (10 - 8))));

	//Now we can start setting up the UART itself
	//Tx enable Rx Enable
	USART1->CR1 |= (1<<3) | (1<<2);

	//Set the baud rate
	//Clock speed should be HCLK (32MHz)
	//Assuming oversample of 8
	//div = 2 * 32M / 9600
	//div = 6667 approx
	USART1->BRR = 6667 & 0x0000FFFF;

	//Enable DMA on RX
	USART1->CR3 |= (1 << 6);
	//Enable the USART
	USART1->CR1 |= (1 << 0);
}
