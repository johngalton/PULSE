/*
 * uart.h
 *
 *  Created on: 7 Jul 2016
 *      Author: ja9g1
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32l0xx_hal.h"

void uart_init(void);
void uart_send(uint8_t *data, uint16_t len);
void uart_handle(void);

#endif /* INC_UART_H_ */
