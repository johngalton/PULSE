/*
 * led_driver.c
 *
 *  Created on: 24 May 2016
 *      Author: John
 */

#include "led_driver.h"

#define nop __asm__("nop")

#define COL_COUNT	8
#define BUF_SIZE	100

#define SYSTICK_ENABLE	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk
#define SYSTICK_DISABLE	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk

typedef struct
{
	uint8_t blue;
	uint8_t red;
	uint8_t green;
	uint8_t dontCare;
} rgb;

typedef union
{
	rgb col;
	uint32_t val;
} colour;

void send_high(uint8_t run);
void send_low(uint8_t run);
void send_res(void);
uint8_t read_buffer(void);
colour get_colour(uint8_t code);

//Buffer for the LED strip we have 150 LED's with 24 bit colour
//B R G X
//8 8 8 8
colour beaconColour;
colour ledStrip[STRIP_SIZE];
colour colour_lookup[COL_COUNT];
uint8_t buffer[BUF_SIZE] = {0};
uint8_t bufferCount = 0;

void led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	//Initialize colour variables
	beaconColour = colour_lookup[0];
	//                     0x00BBGGRR
	colour_lookup[0].val = 0x00000000;//Nothing
	colour_lookup[1].val = 0x000000FF;//Red
	colour_lookup[2].val = 0x0000FFFF;//Yellow
	colour_lookup[3].val = 0x0000FF00;//Green
	colour_lookup[4].val = 0x00FFFF00;//Cyan
	colour_lookup[5].val = 0x00FF0000;//Blue
	colour_lookup[6].val = 0x00FF00FF;//Magenta
	colour_lookup[7].val = 0x00FFFFFF;//White

	//Enable clock for GPIOB
	__HAL_RCC_GPIOB_CLK_ENABLE();

	//Initialise B7 (led data)
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Set it to off by default */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
}

void led_update(void)
{
	SYSTICK_DISABLE;

	uint16_t count = 0;
	uint32_t mask = 0x00800000;

	mask = 0x00800000;
	uint8_t isLast = 0;
	uint32_t maxCount = STRIP_SIZE * 24;
	uint32_t bitCount = 0;
//beacon 48
	uint16_t beaconCount = 48*24;

	do
	{
		isLast = mask & 1;

		if (ledStrip[count].val & mask)
			send_high(isLast);
		else
			send_low(isLast);

		if (isLast)
		{
			mask = 0x00800000;
			count++;
		}
		else
		{
			mask >>= 1;
			nop;
			nop;
		}
		bitCount++;

	} while (bitCount < maxCount);


	bitCount = 0;
	isLast = 0;
	mask = 0x00800000;

	do
	{
		isLast = mask & 1;

		if (beaconColour.val & mask)
			send_high(isLast);
		else
			send_low(isLast);

		if (isLast)
		{
			mask = 0x00800000;
		}
		else
		{
			mask >>= 1;
			nop;
			nop;
		}

		bitCount++;
	} while (bitCount < beaconCount);

	SYSTICK_ENABLE;
}

void led_send_colour(uint32_t col)
{
	uint32_t mask;

	for (mask = 0x00000001; mask < 0x01000000; mask <<= 1)
	{
		if (col & mask)
			send_high(1);
		else
			send_low(1);
	}
}

//Push the value on to the buffer
//Return 1 if success otherwise failure.
uint8_t led_push_buffer(uint8_t value)
{
	if (bufferCount == BUF_SIZE)
		return 0;

	buffer[bufferCount++] = value;
	return 1;
}

void led_set_beacon(uint8_t value)
{
	beaconColour = get_colour(value);
}

void led_propagate(void)
{
	beaconColour.col.red >>= 1;
	beaconColour.col.green >>= 1;
	beaconColour.col.blue >>= 1;

	//Move existing LED's down
	for (uint16_t currentLED = 0; currentLED < (STRIP_SIZE - 1); currentLED++)
	{
		if (ledStrip[currentLED+1].val > 0)
			ledStrip[currentLED].val = ledStrip[currentLED+1].val;
		else if (ledStrip[currentLED].val > 0)
		{
			//ledStrip[currentLED].val = 0;
			ledStrip[currentLED].col.red >>= 1;
			ledStrip[currentLED].col.green >>= 1;
			ledStrip[currentLED].col.blue >>= 1;
		}
		else
			ledStrip[currentLED].val = 0;
	}

	ledStrip[STRIP_SIZE-1] = get_colour(read_buffer());
}

void send_high(uint8_t run)
{
	//Use scope to calculate number of nops
	GPIOB->BSRR = GPIO_PIN_7;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	GPIOB->BRR = GPIO_PIN_7;
	if (run)
	{
		nop;
		nop;
		nop;
		nop;
		nop;
		nop;
	}
}

void send_low(uint8_t run)
{
	//Use scope to calculate number of nops
	GPIOB->BSRR = GPIO_PIN_7;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	GPIOB->BRR = GPIO_PIN_7;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	if (run)
	{
		nop;
		nop;
		nop;
		nop;
		nop;
		nop;
	}
}

void send_res(void)
{
	//Use scope to calculate number of nop

	GPIOB->BRR = GPIO_PIN_5;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
	nop;
}

uint8_t read_buffer(void)
{
	if (bufferCount == 0)
		return 0;

	uint8_t value = buffer[0];

	for (uint16_t count = 0; count < bufferCount; count++)
	{
		buffer[count] = buffer[count+1];
	}

	bufferCount--;

	return value;
}

colour get_colour(uint8_t code)
{
	if (code < COL_COUNT)
		return colour_lookup[code];
	else
		return colour_lookup[0];
}

