/**********************************************************
*
*    Filename     pole.h
*    Author       A Cowan
*    Start date   26/01/2018
*    Project      PULSE
*    Description  Class used for communicating with the LED
*                 poles in the PULSE setup. One instance of this
*                 class can communicate with all of the poles.
*
**********************************************************/

#ifndef pole_h
#define pole_h

#include "Arduino.h"

enum blockColour
{
	COLOUR_OFF = 0,
	COLOUR_RED,
	COLOUR_YEL,
	COLOUR_GRE,
	COLOUR_CYA,
	COLOUR_BLU,
	COLOUR_MAG,
	COLOUR_WHI
};

enum poleID
{
	POLE1_ID = 0x01,
	POLE2_ID = 0x02,
	POLE3_ID = 0x04,
	POLES123_ID = 0x07	//broadcast ID
};

enum scrollDirection
{
	SCROLL_DOWN = 0,
	SCROLL_UP
};

struct ledBlock
{
	uint8_t blockColour[3];	//colours for each of the three poles
	uint8_t blockSize;
};

class pole
{
public:
	// Class variables
	uint16_t updateSpeedMs;
	uint16_t poleDelayMs;

	// Class functions
	pole();                    //constructor
	bool initialise();
  bool addNoteBlock(uint8_t poles, uint8_t duration);
	bool addLedBlock(ledBlock blockData);
	bool setUpdateSpeed(uint16_t updateSpeed);
	bool setBeaconFade(uint8_t poleAddress, uint8_t fadeSpeed);
	bool setScrollDirection(uint8_t poleAddress, uint8_t direction);

private:
	// Class variables

	// Class functions

};

#endif
