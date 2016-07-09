/*
 * uart.c
 *
 *  Created on: 7 Jul 2016
 *      Author: ja9g1
 */

#include "uart.h"

#define writePosition (128 - (uint8_t)DMA1_Channel3->CNDTR)
#define bytesToRead ((128 + (writePosition - readPosition)) % 128)
#define bytesToEcho ((128 + (writePosition - echoPosition)) % 128)
#define TXE_Set 	USART1->CR1 |= (1<<3)
#define TXE_Clear	USART1->CR1 &= ~(1<<3)
#define TXE			((USART1->CR1 >> 3) & 1)
#define RXE_Set 	USART1->CR1 |= (1<<2)
#define RXE			((USART1->CR1 >> 2) & 1)

//129 is only a precaution for now as I'm not 100% sure if DMA count is ever at 0
uint8_t rxBuffer[129];

uint8_t readPosition = 0;
uint8_t echoPosition = 0;

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
	TXE_Set;
	RXE_Set;

	//Set the baud rate
	//Clock speed should be HCLK (32MHz)
	//Assuming oversample of 8
	//div = 2 * 32M / 9600
	//div = 6667 approx
	USART1->BRR = 6667 & 0x0000FFFF;

	//0010 0000 1010 0000
	DMA1_Channel3->CCR = 0x20A0;
	//Buffer size
	DMA1_Channel3->CNDTR = 128;
	//Peripheral address is the usart1 read data register
	DMA1_Channel3->CPAR = (uint32_t)&USART1->RDR;
	//Memory address is the receive buffer
	DMA1_Channel3->CMAR = (uint32_t)&rxBuffer;

	//Select USART1 as the input for DMA3
	DMA1_CSELR->CSELR |= (0x3 << ((3 - 1) * 4));
	DMA1_CSELR->CSELR &= ~(0x0C << ((3 - 1) * 4));

	//Enable DMA on RX
	USART1->CR3 |= (1 << 6);
	//Enable the USART
	USART1->CR1 |= (1 << 0);
}

void uart_send(uint8_t *data, uint16_t len)
{
	for (uint16_t count = 0; count <= len; count++)
	{
		//Load data in to the send register
		USART1->TDR = data[count];
		//Wait for the TXE to be set
		while (TXE == 0) ;
	}

	//Wait for Transfer Complete
	while (((USART1->ISR >> 6) & 1) == 0) ;
}

void uart_handle(void)
{
	//Echo any necessary bytes
	while (bytesToEcho > 0)
	{
		uart_send(&rxBuffer[echoPosition], 1);
		echoPosition = (echoPosition + 1) % 128;
	}

	//Process necessary data
	while (bytesToRead > 0)
	{
		if (rxBuffer[readPosition] == 0xFE)
		{

		}
	}
}
