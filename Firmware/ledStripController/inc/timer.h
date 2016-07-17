/*
 * timer.h
 *
 *  Created on: 5 Jul 2016
 *      Author: ja9g1
 */


#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "stm32l0xx_hal.h"
#include "led_driver.h"
#include "uart.h"

volatile uint8_t update_led;

typedef enum {timer_enabled, timer_disabled} timerState;

void timer_init(void (*handler)(void));
void timer_delay_ms(uint32_t delay);
void timer_set_interval(uint16_t interval);
uint16_t timer_get_interval(void);
uint32_t timer_get_now(void);
void timer_set_state(timerState newState);
timerState timer_get_state(void);
void SysTick_Handler(void);
void TIM2_IRQHandler(void);


#endif /* INC_TIMER_H_ */
