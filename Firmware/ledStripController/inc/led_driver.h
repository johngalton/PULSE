/*
 * led_driver.h
 *
 *  Created on: 24 May 2016
 *      Author: John
 */

#include "stm32l0xx_hal.h"

#ifndef INC_LED_DRIVER_H_
#define INC_LED_DRIVER_H_

#define STRIP_SIZE	200

void led_init(void);
void led_update(void);
void led_send_colour(uint32_t col);
uint8_t led_push_buffer(uint8_t value);
void led_clear_buffer(void);
void led_clear_strip(void);
void led_set_beacon(uint8_t value);
void led_set_beacon_background(uint8_t value);
void led_set_beacon_fade_div(uint8_t value);
void led_pulse_target(uint8_t code);
void led_hide_target(uint8_t hide);
void led_propagate(void);
void led_set_all(uint8_t code);
void led_show_logo(uint8_t code, uint8_t offset);
void led_invert_direction(uint8_t invert);
void led_display_id(uint8_t id);

#endif /* INC_LED_DRIVER_H_ */
