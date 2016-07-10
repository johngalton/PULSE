/*
 * uart.h
 *
 *  Created on: 7 Jul 2016
 *      Author: ja9g1
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32l0xx_hal.h"
#include "debug.h"
#include "timer.h"
#include <stdlib.h>

typedef struct
{
	uint8_t address;
	uint8_t command;
	uint8_t length;
	uint8_t *data;
	uint8_t end;
} uart_long_function;

void uart_init(void (*shortHandler)(uint8_t *), void (*longHandler)(uart_long_function));
void uart_send(uint8_t *data, uint16_t len);
void uart_handle(void);

#endif /* INC_UART_H_ */
