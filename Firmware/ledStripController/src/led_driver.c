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

#define TARGET_COL	0x00FFFFFF

#define SYSTICK_ENABLE	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk
#define SYSTICK_DISABLE	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk
#define GET_POS(x)		((x + bufferPosition) % STRIP_SIZE)


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
uint32_t mod(uint32_t numerator, uint32_t denominator);

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
uint8_t bufferPosition = 0;
colour targetColour;
uint8_t notePressed = 0;
uint8_t beaconSet = 0;
uint8_t invertDirection = 0;
uint8_t targetHidden = 0;

colour targetColourDefault;

#if (DEV_ID==1)
const uint8_t logo[29] = {1,1,1,1,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1};
#elif (DEV_ID==2)
const uint8_t logo[29] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1};
#else
const uint8_t logo[29] = {1,0,1,0,1,0,1,1,1,0,1,0,1,0,0,0,0,0,1,1,1,1,1,0,0,0,1,1,1};
#endif

void led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	//Initialize colour variables
	beaconColour = colour_lookup[0];
	beaconBackground = colour_lookup[0];
	targetColour.val = targetColourDefault.val = TARGET_COL;

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

	//Initialise B7 (led data) and B4 (Enable)
	GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Set it to off by default */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
}

void led_update(void)
{
	SYSTICK_DISABLE;

	uint16_t count = bufferPosition;
	uint32_t mask = 0x00800000;

	mask = 0x00800000;
	uint8_t isLast = 0;
	uint32_t maxCount = STRIP_SIZE * 24;
	uint32_t bitCount = 0;
//beacon 48
	uint16_t beaconCount = 48*24;
	uint8_t stripEnd = STRIP_SIZE-1;
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
			if (count == stripEnd)
				count = 0;
			else
				count++;
		}
		else
		{
			mask >>= 1;
			//nop;
			//nop;
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
			//nop;
			//nop;
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

void led_clear_buffer(void)
{
	memset(buffer,0,sizeof(buffer));
}

void led_clear_strip(void)
{
	memset(ledStrip, 0, sizeof(ledStrip));
}

void led_set_beacon(uint8_t value)
{
	beaconColour = get_colour(value);
	beaconSet = 1;

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

void led_pulse_target(uint8_t code)
{
	notePressed = 1;
}

void led_hide_target(uint8_t hide)
{
	if (hide == 0)
		targetHidden = 0;
	else
		targetHidden = 1;
}

void led_set_beacon_fade_div(uint8_t value)
{
	beaconFadeDiv = value;
}

void led_show_logo(uint8_t code, uint8_t offset)
{
	uint8_t position = STRIP_SIZE - 29 - offset;
	colour logoCol = get_colour(code);

	for (uint8_t count = 0; count < 29; count++)
	{
		if (logo[count] == 1)
			ledStrip[GET_POS(position+count)] = logoCol;
		else
			ledStrip[GET_POS(position+count)] = get_colour(0);
	}
}

void led_invert_direction(uint8_t invert)
{
	if (invert == 0)
		invertDirection = 0;
	else
		invertDirection = 1;
}

void led_propagate(void)
{
	if (targetColour.val != TARGET_COL && notePressed == 0)
	{
		targetColour.col.red <<= 1;
		targetColour.col.green <<= 1;
		targetColour.col.blue <<= 1;

		targetColour.val |= 0x00010101;
	}

	if (targetHidden == 0)
	{
		ledStrip[GET_POS(10)].val = 0;
		ledStrip[GET_POS(9)].val = 0;
	}

	//Move existing LED's down
 	if (invertDirection)
 	{
 		bufferPosition = mod((bufferPosition + 1), STRIP_SIZE);
 	}
 	else
 	{
 		bufferPosition = mod((bufferPosition - 1), STRIP_SIZE);
 	}

	uint16_t currentLED = GET_POS(1);
	uint16_t previousLED = bufferPosition; //0

	if (invertDirection)
	{
		currentLED = GET_POS(STRIP_SIZE-2);
		previousLED = GET_POS(STRIP_SIZE-1);
	}

	for (uint16_t ledCount = 0; ledCount < (STRIP_SIZE - 1); ledCount++)
	{
		if (ledStrip[currentLED].val == 0 && ledStrip[previousLED].val > 0)
		{
			ledStrip[currentLED] = ledStrip[previousLED];

			ledStrip[currentLED].col.red >>= 1;
			ledStrip[currentLED].col.green >>= 1;
			ledStrip[currentLED].col.blue >>= 1;
		}

		if (invertDirection)
		{
			currentLED = mod(currentLED-1, STRIP_SIZE);
			previousLED = mod(previousLED-1, STRIP_SIZE);
		}
		else
		{
			currentLED = (currentLED+1) % STRIP_SIZE;
			previousLED = (previousLED+1) % STRIP_SIZE;
		}
	}

	if (invertDirection)
	{
		ledStrip[bufferPosition] = get_colour(read_buffer());
	}
	else
	{
		ledStrip[GET_POS(STRIP_SIZE-1)] = get_colour(read_buffer());
	}

	if (ledStrip[GET_POS(STRIP_SIZE-1)].val == 0)
	{
		beaconSet = 0;
	}

	if (beaconBackground.val != beaconColour.val && beaconSet == 0)
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

	if (targetHidden == 0)
	{
		uint8_t afterTargetPos = GET_POS(8);
		ledStrip[afterTargetPos] = ledStrip[GET_POS(10)];

		//If the note has been hit we should fade the data or clear it completely
		if (notePressed)
		{
			if (ledStrip[afterTargetPos].val == 0)
			{
				if (notePressed == 2)
					notePressed = 0;
			}
			else if (notePressed == 1)
			{
				targetColour = ledStrip[afterTargetPos];
				notePressed = 2;
			}

			ledStrip[afterTargetPos].val = 0;
			ledStrip[GET_POS(7)].val = 0;
			ledStrip[GET_POS(6)].val = 0;
			ledStrip[GET_POS(5)].val = 0;
			ledStrip[GET_POS(4)].val = 0;
			ledStrip[GET_POS(3)].val = 0;
			ledStrip[GET_POS(2)].val = 0;
			ledStrip[GET_POS(1)].val = 0;
			ledStrip[GET_POS(0)].val = 0;
			//ledStrip[afterTargetPos].col.red >>= 4;
			//ledStrip[afterTargetPos].col.green >>= 4;
			//ledStrip[afterTargetPos].col.blue >>= 4;
		}

		ledStrip[GET_POS(10)] = targetColour;//0x00101010;
		ledStrip[GET_POS(9)] = targetColour;//0x00101010;
	}
}

void send_high(uint8_t run)
{
	//Use scope to calculate number of nops
	GPIOB->BSRR = GPIO_PIN_7;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
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
		//nop;
		//nop;
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
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	//nop;
	if (run)
	{
		//nop;
		//nop;
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

uint32_t mod(uint32_t numerator, uint32_t denominator)
{
	uint32_t value = numerator % denominator;

	if (numerator < 0)
	{
		value += denominator;
	}

	return value;
}
