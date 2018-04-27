/**********************************************************
*
*    Filename     track.h
*    Author       A Cowan
*    Start date   30/11/2017
*    Project      PULSE
*    Description  Class representing one of the
*                 possible tracks that could be
*                 selected for a PULSE game
*
**********************************************************/

#ifndef track_h
#define track_h

#include "Arduino.h"
#include "calendar.h"
#include "note.h"
#include "midi.h"
#include "VS1053.h"
#include <SPI.h>
#include <SD.h>

extern VS1053 audioCodec;

#define INI_MAX_LINE 64

//used for 'parseStatus' variable		//  OGG_found MID_found x delay_found   title_found artist_found INI_found folder_found
#define PARSE_OGG_FOUND	0x80
#define PARSE_MID_FOUND	0x40
#define PARSE_TOP_FOUND 0x20
#define PARSE_DEL_FOUND	0x10
#define PARSE_TTL_FOUND	0x08
#define PARSE_ART_FOUND	0x04
#define PARSE_INI_FOUND	0x02
#define PARSE_FDR_FOUND	0x01
#define PARSE_CLEARED	0x00

extern note calendar[];

class track
{
public:
	// Class variables
	uint8_t track_ID;				//also position in songbook array
	char songPath[24];
	uint8_t parseStatus;    //  OGG_found MID_found x delay_found   title_found artist_found INI_found folder_found
	uint16_t numberNotes;
	uint32_t ogg_duration;
	uint16_t midi_offset;
	char* artist;
	char* title;
	uint8_t presetLowestNote;

	// Class functions
	track();                    //constructor
	uint8_t playCountdown(byte volume);    //plays the countdown track
	uint8_t playOgg(byte volume);					//determines the length, and validity
	uint8_t parseIni();					//reads associated parameters
	uint8_t parseMidi();				//takes a filename, populates the global calendar (which comes from an extern)

	char* getArtist();
	char* getTitle();
	bool iniLoaded();

private:
	// Class variables

	// Class functions

};

#endif
