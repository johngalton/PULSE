/**********************************************************
*
*    Filename     midi.h
*    Author       A Cowan
*    Start date   02/12/2017
*    Project      PULSE
*    Description  Class representing a midi
*                 file to be decoded
*
**********************************************************/

#ifndef midi_h
#define midi_h

#include "Arduino.h"
#include "calendar.h"
#include "note.h"
#include <SPI.h>
#include <SD.h>
#include "errorCodes.h"

//used for 'contents' variable		//  ERROR x x OTHER END LYRICS NOTES TEMPO
#define CONTENTS_ERROR	0x80
#define CONTENTS_OTHER	0x10
#define CONTENTS_END	0x08
#define CONTENTS_LYRICS	0x04
#define CONTENTS_NOTES	0x02
#define CONTENTS_TEMPO	0x01
#define CONTENTS_NONE	0x00

extern noteCalendar noteList;

class midi
{
	struct track
	{
		uint32_t trackLength;
		uint32_t startOffset;
		bool loaded;
		uint8_t contents;
		uint16_t noteCount;
		uint16_t tempoChanges;
		uint8_t presetLowestNote; //if 0, then uses automatic mode
		uint8_t highestNote;
		uint8_t lowestNote;
		uint16_t division;
	};

public:
	// Class variables
	char* filename;

	uint8_t numberOfTracks;
	track midiTrack[4];
	uint16_t midi_offset;

	// Class functions
	midi();   //constructor
	midi(char* inFilename);   //constructor
	uint8_t decodeHeader(void);
	uint8_t scanTracks();
	uint8_t populateCalendar(uint8_t presetNote);
	uint8_t computeTimings(uint16_t in_midi_offset);
	uint8_t decodeMidiTrack(uint8_t trackID, bool populateNotes, bool computeTimings);

private:
	// Class variables

	// Class functions

};

#endif
