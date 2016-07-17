/*
 * led_driver.c
 *
 *  Created on: 24 May 2016
 *      Author: John
 */

#include "led_driver.h"

#define nop __asm__("nop")

#define COL_COUNT	8
#define BUF_SIZE	1000

#define SYSTICK_ENABLE	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk
#define SYSTICK_DISABLE	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk

typedef struct
{
	uint8_t green;
	uint8_t red;
	uint8_t blue;
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
colour beaconBackground;
uint8_t beaconMask = 0;
uint8_t beaconFadeDiv = 0x10;
int8_t beaconRedStep;
int8_t beaconGreenStep;
int8_t beaconBlueStep;
colour ledStrip[STRIP_SIZE];
colour colour_lookup[COL_COUNT];
uint8_t buffer[BUF_SIZE] = {0};
uint16_t bufferCount = 0;
colour targetColour;

colour targetColourDefault;
void led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	//Initialize colour variables
	beaconColour = colour_lookup[0];
	beaconBackground = colour_lookup[0];
	targetColour.val = targetColourDefault.val = 0x00101010;

	//					   0x00GGRRBB
	colour_lookup[0].val = 0x00000000;//Nothing
	colour_lookup[1].val = 0x0000FF00;//Red
	colour_lookup[2].val = 0x00FFFF00;//Yellow
	colour_lookup[3].val = 0x00FF0000;//Green
	colour_lookup[4].val = 0x00FF00FF;//Cyan
	colour_lookup[5].val = 0x000000FF;//Blue
	colour_lookup[6].val = 0x0000FFC0;//Magenta
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
	if (value == 0)
		return 1;

	buffer[bufferCount++] = value;
	return 1;
}

void led_set_beacon(uint8_t value)
{
	beaconColour = get_colour(value);

	beaconBlueStep = ((beaconBackground.col.blue - beaconColour.col.blue) / beaconFadeDiv) + 1;
	beaconGreenStep = ((beaconBackground.col.green - beaconColour.col.green) / beaconFadeDiv) + 1;
	beaconRedStep = ((beaconBackground.col.red - beaconColour.col.red) / beaconFadeDiv) + 1;
}

void led_set_beacon_background(uint8_t value)
{
	beaconBackground = get_colour(value);
	beaconBackground.col.red >>= 5;
	beaconBackground.col.green >>= 5;
	beaconBackground.col.blue >>= 5;

	beaconBlueStep = ((beaconBackground.col.blue - beaconColour.col.blue) / beaconFadeDiv) + 1;
	beaconGreenStep = ((beaconBackground.col.green - beaconColour.col.green) / beaconFadeDiv) + 1;
	beaconRedStep = ((beaconBackground.col.red - beaconColour.col.red) / beaconFadeDiv) + 1;
}

void led_pulse_target(void)
{
	targetColour.val = 0x00FFFFFF;
}

void led_set_beacon_fade_div(uint8_t value)
{
	beaconFadeDiv = value;
}

void led_propagate(void)
{
	if (targetColour.val != 0x00101010)
	{
		targetColour.col.red >>= 1;
		targetColour.col.green >>= 1;
		targetColour.col.blue >>= 1;

		if (targetColour.col.red < 0x10)
		{
			targetColour.val = 0x00101010;
		}
	}

	if (beaconBackground.val != beaconColour.val)
	{
		int16_t tmp = beaconColour.col.red + beaconRedStep;

		if (((tmp > beaconBackground.col.red) && beaconRedStep > 0) || ((tmp < beaconBackground.col.red) && beaconRedStep < 0))
			beaconColour.col.red = beaconBackground.col.red;
		else
			beaconColour.col.red = tmp;


		tmp = beaconColour.col.green + beaconGreenStep;

		if (((tmp > beaconBackground.col.green) && beaconGreenStep > 0) || ((tmp < beaconBackground.col.green) && beaconGreenStep < 0))
			beaconColour.col.green = beaconBackground.col.green;
		else
			beaconColour.col.green = tmp;

		tmp = beaconColour.col.blue + beaconBlueStep;

		if (((tmp > beaconBackground.col.blue) && beaconBlueStep > 0) || ((tmp < beaconBackground.col.blue) && beaconBlueStep < 0))
			beaconColour.col.blue = beaconBackground.col.blue;
		else
			beaconColour.col.blue = tmp;
	}

	ledStrip[10].val = 0;
 	ledStrip[9].val = 0;

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
	}

	ledStrip[STRIP_SIZE-1] = get_colour(read_buffer());

	ledStrip[8] = ledStrip[10];
	ledStrip[10] = targetColour;//0x00101010;
	ledStrip[9] = targetColour;//0x00101010;
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

void led_set_all(uint8_t code)
{
	colour selected = get_colour(code);
	for (uint8_t count = 0; count < STRIP_SIZE; count++)
	{
		ledStrip[count] = selected;
	}
}
