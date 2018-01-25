/**********************************************************
*
*    Filename     note.cpp
*    Author       A Cowan
*    Start date   30/11/2017
*    Project      PULSE
*    Description  Class representing a single note
*                 (combination) in a PULSE song
*
**********************************************************/

#include "Arduino.h"
#include "note.h"


note::note()		//constructor
{
	timestamp = 0;
	event = 0;
	duration = 0;
	configured = false;
}

/**
*	\brief Resets a note's parameters
*
*	\return None
**/
void note::reset()
{
	timestamp = 0;
	event = 0;
	duration = 0;
	configured = false;
}
