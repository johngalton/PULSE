/***************************************
*
*    Filename     calendar.h
*    Author       A Cowan
*    Start date   04/12/2017
*    Project      PULSE
*    Description  Class representing the notes
*                 of an entire PULSE song
*
**********************************************************/

#ifndef calendar_h
#define calendar_h

#include "Arduino.h"
#include "note.h"

#define CALENDAR_MAX_SIZE 500

class noteCalendar
{
public:
	note noteArray[CALENDAR_MAX_SIZE];   //array of midi notes, populated by whatever song is needed
	uint16_t totalNotes;
	int16_t tempoSetPtr;		//used to indicate which times are in ticks, and which are in msec
	int16_t playbackPtr;		//used to indicate the next note to play
	note currentNote;

	noteCalendar();
	void reset(void);
	void addNote(note newNote);
	bool getNote(uint32_t currentTimestamp);
	void serialPrint(uint16_t start, uint16_t length);

private:
};
#endif
