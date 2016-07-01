/*
 * debug.h
 *
 *  Created on: 2 Jun 2016
 *      Author: John
 */

#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#include "stm32l0xx_hal.h"

#define DEBUG_Y_LED_ON	GPIOB->BSRR = GPIO_PIN_0
#define DEBUG_Y_LED_OFF	GPIOB->BRR = GPIO_PIN_0
#define DEBUG_G_LED_ON	GPIOB->BSRR = GPIO_PIN_1
#define DEBUG_G_LED_OFF	GPIOB->BRR = GPIO_PIN_1

void debug_init(void);
void debug_disable_usart(void);
void debug_enable_usart(void);

#endif /* INC_DEBUG_H_ */
