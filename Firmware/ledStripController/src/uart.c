/*
 * uart.c
 *
 *  Created on: 7 Jul 2016
 *      Author: ja9g1
 */

#include "uart.h"

#define writePosition 	(128 - (uint8_t)DMA1_Channel3->CNDTR)
#define bytesToRead 	((128 + (writePosition - readPosition)) % 128)
#define TXE_Set 		USART1->CR1 |= (1<<3)
#define TXE_Clear		USART1->CR1 &= ~(1<<3)
#define TXE				(USART1->ISR & (1 << 7))
#define RXE_Set 		USART1->CR1 |= (1<<2)
#define RXE				((USART1->CR1 >> 2) & 1)
#define MAX_READ_BYTES	20

typedef enum {idle, shortCmd, longCmd1, longCmd2} state;

state currentState = idle;

//129 is only a precaution for now as I'm not 100% sure if DMA count is ever at 0
uint8_t rxBuffer[128];
uint8_t readPosition = 0;
uint32_t lastDataTime = 0;

uint8_t readBuffer(void);

void (*shortCmdHandler)(uint8_t *);
void (*longCmdHandler)(uart_long_function);


uart_long_function currentFunc;

void uart_init(void (*shortHandler)(uint8_t *), void (*longHandler)(uart_long_function))
{
	shortCmdHandler = shortHandler;
	longCmdHandler = longHandler;

	//Enable clock for GPIOA
	RCC->IOPENR |= (1 << 0);
	//Enable clock for USART1
	RCC->APB2ENR |= (1 << 14);
	//DMA CLK
	RCC->AHBENR |= (1 << 0);

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
	//TXE_Set;
	//RXE_Set;

	//Set the baud rate
	//Clock speed should be HCLK (32MHz)
	//Assuming oversample of 16
	//div = 32M / 9600
	//div = 3333 (0xD05) approx
	USART1->BRR = 0xD05;//6667 & 0x0000FFFF;

	//Ensure 8 data bits
	USART1->CR1 &= ~((1 << 28) | (1 << 12));
	USART1->CR1 |= (1 << 15);

	//0000 0000 0000 0000 0000 0000 0000 0000
	//00000000
	USART1->CR2 = 0x00000000;

	//0000 0000 0000 0000 0000 0000 0100 0000
	//000000C0
	USART1->CR3 = 0x00000040;

	//0000 0000 0000 0000 0000 0000 0000 1100
	//00000008
	USART1->CR1 = 0x0000000A;

	RXE_Set;

	USART1->CR1 |= 1;

	//Select USART1_RX as the input for DMA3
	DMA1_CSELR->CSELR = 0x00000300;
	//0010 0000 1010 0000
	DMA1_Channel3->CCR = 0x20A0;
	//Peripheral address is the usart1 read data register
	DMA1_Channel3->CPAR = (uint32_t)&USART1->RDR;
	//Memory address is the receive buffer
	DMA1_Channel3->CMAR = (uint32_t)rxBuffer;
	//Buffer size
	DMA1_Channel3->CNDTR = 128;

	DMA1_Channel3->CCR |= 1;

}

void uart_send(uint8_t *data, uint16_t len)
{
	//DEBUG_Y_LED_ON;
	for (uint16_t count = 0; count < len; count++)
	{
		USART1->TDR = data[count];

		while (TXE == 0) ;
	}

	//Wait for Transfer Complete
	while (((USART1->ISR >> 6) & 1) == 0) ;

	//DEBUG_Y_LED_OFF;
}

void uart_handle(void)
{
	if (bytesToRead > 0)
	{
		DEBUG_Y_LED_ON;
		lastDataTime = timer_get_now();
	}
	else if (timer_get_now() > (lastDataTime + 200))
	{
		DEBUG_Y_LED_OFF;
	}

	switch (currentState)
	{
		case idle:
		{
			uint8_t byte = readBuffer();
			if (byte == 0xFE)
			{
				uart_send((uint8_t *)"OK",2);
				currentState = shortCmd;
				break;
			}
			else if (byte == 0xFC)
			{
				uart_send((uint8_t *)"OK",2);
				currentState = longCmd1;
				break;
			}
		}
		break;
		case shortCmd:
			if (bytesToRead >= 7)
			{
				//uart_send((uint8_t *)"OK",2);
				uint8_t data[7] = {0};
				for (uint8_t count = 0; count < 7; count++)
				{
					data[count] = readBuffer();
				}
				shortCmdHandler(data);
				currentState = idle;
			}
		break;
		case longCmd1:
			if (bytesToRead >= 3)
			{
				//uart_send((uint8_t *)"OK",2);
				currentFunc.address = readBuffer();
				currentFunc.command = readBuffer();
				currentFunc.length = readBuffer();

				currentFunc.data = malloc(currentFunc.length);
				currentState = longCmd2;
			}
		break;
		case longCmd2:
		{
			if (bytesToRead >= currentFunc.length + 1)
			{
				for (uint8_t count = 0; count < currentFunc.length; count++)
				{
					currentFunc.data[count] = readBuffer();
				}

				currentFunc.end = readBuffer();

				longCmdHandler(currentFunc);

				free(currentFunc.data);
				currentState = idle;
			}
		}
		break;
		default: break;
	}
}

uint8_t readBuffer(void)
{
	if (readPosition == writePosition)
		return 0x00;

	uint8_t value = rxBuffer[readPosition];

	readPosition = (readPosition + 1) % 128;

	return value;
}
