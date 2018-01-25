/**********************************************************
*
*    Filename     audioLib.h
*    Author       A Cowan
*    Start date   30/11/2017
*    Project      PULSE
*    Description  Collection of library functions
*                 for handling the song book
*
**********************************************************/

#ifndef audioLib_h
#define audioLib_h

#include "Arduino.h"
#include "calendar.h"
#include "track.h"
#include "note.h"
#include "errorCodes.h"
#include <SPI.h>
#include <SD.h>

class audioLib
{
public:
	// Class variables
	track* songbook;
	int numberSongs;			//length of songbook)

// Class functions
	audioLib(void);					//constructor
	int initSDcard(void);			//initilises SD card
	int scanSDcard(void);			//scans SD card, creates array of songs with size of number of directories with the right file names in (uses malloc, only ever runs on startup)
	int parseSDcard(void);			//for each song, reads in the files and populates the fields
	int printSongbook(void);

private:
	// Class variables
	bool scanComplete;			//to make sure we don't create more than one array
	bool parseComplete;			//to make sure we do run this before attempting to populate fields

// Class functions

};

#endif
