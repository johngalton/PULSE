/**********************************************************
*
*    Filename     note.h
*    Author       A Cowan
*    Start date   30/11/2017
*    Project      PULSE
*    Description  Class representing a single note
*                 (combination) in a PULSE song
*
**********************************************************/

#ifndef note_h
#define note_h

#include "Arduino.h"

class note
{
public:
	// Class variables
	uint32_t timestamp;		//in milliseconds
	uint8_t event;			//bits 0-4 are the buttons, 5-7 are reserved
	uint16_t duration;
	bool configured;

	// Class functions
	note();		//constructor
	void reset();

private:
	// Class variables

	// Class functions

};

#endif
