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

#include "Arduino.h"
#include "pole.h"

pole::pole()    //constructor
{
}

/**
*	\brief Initialises the pole control class
*
*	\return An boolean representing success
**/
bool pole::initialise()
{
	Serial1.begin(9600);

	return true;
}

/**
*  \brief Adds a coloured block to a pole (or poles)
*
* Commands one or more poles to add a coloured block to the end
*
* \return An boolean representing success
**/
bool pole::addNoteBlock(uint8_t poles, uint8_t duration)
{
  ledBlock currentNote;
  if (poles & 0x01)
    currentNote.blockColour[0] = COLOUR_RED;
  else if (poles & 0x02)
    currentNote.blockColour[0] = COLOUR_YEL;
  else
    currentNote.blockColour[0] = COLOUR_OFF;

  if (poles & 0x04)
    currentNote.blockColour[1] = COLOUR_GRE;
  else
    currentNote.blockColour[1] = COLOUR_OFF;

  if (poles & 0x08)
    currentNote.blockColour[2] = COLOUR_BLU;
  else if (poles & 0x10)
    currentNote.blockColour[2] = COLOUR_MAG;
  else
    currentNote.blockColour[2] = COLOUR_OFF;

  currentNote.blockSize = duration; 
  return addLedBlock(currentNote);
}

/**
*	\brief Adds a coloured block to a pole (or poles)
*
*	Commands one or more poles to add a coloured block to the end
*
*	\return An boolean representing success
**/
bool pole::addLedBlock(ledBlock blockData)
{
	uint8_t outData[9];
	
	outData[0] = 0xFE;	//Update
	outData[1] = 0x00;	//Add LED at top of pole

	for (int poleNum = 0; poleNum < 3; poleNum++)
	{
		if (blockData.blockColour[poleNum] == COLOUR_OFF)
		{
			outData[2 + (poleNum * 2)] = 0x00;		//if that pole isn't needed, set block size and colour to 0
			outData[3 + (poleNum * 2)] = 0x00;
		}
		else
		{
			outData[2 + (poleNum * 2)] = blockData.blockColour[poleNum];
			outData[3 + (poleNum * 2)] = blockData.blockSize;
		}
	}

	outData[8] = 0x00;	//checksum (pfft, who needs them)
  
	Serial1.write(outData, 9);

	return true;
}

/**
*	\brief Sets the update speed of the poles
*
*	Sets the update speed of the poles, in milliseconds per LED.
*	The delay of the entire pole is equal to updateSpeed * 150.
*	Note this is always sent as a broadcast command
*
*	\return An boolean representing success
**/
bool pole::setUpdateSpeed(uint16_t updateSpeed)
{
	updateSpeedMs = updateSpeed;
	poleDelayMs = updateSpeedMs * 194;		//TODO - the LED controller code says poleDelay=update*150. The python code has update*178. Which is right?
											                  //200 LEDs per strip, is target line is up by 22?
                                       //Update 26/05/2018: 194 seems to give good results? 

	uint8_t outData[6];
 
  outData[0] = 0xFC;    // Start
  outData[1] = 0x00;    // Broadcast address
  outData[2] = 0x01;    // Set update speed
  outData[3] = 0x02;    // Two data bytes in command
  outData[4] = (updateSpeed & 0xFF00);    // High byte
  outData[5] = (updateSpeed & 0x00FF);    // Low byte
  outData[6] = 0xFD;    // End

	Serial1.write(outData, 7);

	return true;
}

/**
*	\brief Sets the fade time of the beacons
*
*	Sets the time taken for beacons to fade to off after activating
*	Fade unit is number of cycles required.
*
*	\return An boolean representing success
**/
bool pole::setBeaconFade(uint8_t poleAddress, uint8_t fadeSpeed)
{
	Serial.println("setBeaconFade not implemented");
	return false;
}

/**
*	\brief Set the pole scroll direction
*
*	Allows the pole scroll direction to be reversed (for startup effects).
*	Reversing the direction will also hide the white 'target' lights.
*
*	NOTE: ADDRESS CAN ONLY BE ONE OR ALL POLES (NO COMBINATIONS)
*
*	\return An boolean representing success
**/
bool pole::setScrollDirection(uint8_t poleAddress, uint8_t direction)
{
	uint8_t outAddress;
	
	if (poleAddress == POLE1_ID)
		outAddress = 1;
	else if (poleAddress == POLE2_ID)
		outAddress = 2;
	else if (poleAddress == POLE3_ID)
		outAddress = 3;
	else if (poleAddress == POLES123_ID)
		outAddress = 0;
	else
	{
		Serial.println("Unsupported scroll ID");
		return false;
	}

	uint8_t outData[6];

	outData[0] = 0xFC;			// Start
	outData[1] = outAddress;	// Pole address
	outData[2] = 0x08;			// Set scroll direction
	outData[3] = 0x01;			// One data byte in command
	outData[4] = direction;		// Scroll direction
	outData[5] = 0xFD;			// End

  Serial1.write(outData, 6);
  
	return true;
}


