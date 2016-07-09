/*
 * uart.h
 *
 *  Created on: 7 Jul 2016
 *      Author: ja9g1
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32l0xx_hal.h"

typedef struct
{
	uint8_t type;
	uint8_t led1;
	uint8_t count1;
	uint8_t led2;
	uint8_t count2;
	uint8_t led3;
	uint8_t count3;
	uint8_t checksum;
} uart_quick_function;

typedef struct
{
	uint8_t address;
	uint8_t command;
	uint8_t length;
	uint8_t *data;
} uart_std_function;

void uart_init(void);
void uart_handle(void);

#endif /* INC_UART_H_ */
