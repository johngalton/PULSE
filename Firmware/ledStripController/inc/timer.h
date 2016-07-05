/*
 * timer.h
 *
 *  Created on: 5 Jul 2016
 *      Author: ja9g1
 */


#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "stm32l0xx_hal.h"

typedef enum {timer_enabled, timer_disabled} timerState;

void timer_init(void);
void timer_delay_ms(uint32_t delay);
void timer_set_interval(uint32_t interval);
void timer_set_state(timerState newState);
timerState timer_get_state(void);
void SysTick_Handler(void);


#endif /* INC_TIMER_H_ */
