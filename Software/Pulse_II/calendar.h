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

struct noteEvent
{
	uint16_t noteNumber;	//the number in the song (used for tracking if we missed one, as this should go up one each time)
	note noteDetail;
	bool isValid;
};

class noteCalendar
{
public:
	note noteArray[CALENDAR_MAX_SIZE];   //array of midi notes, populated by whatever song is needed
	uint16_t totalNotes;
	int16_t tempoSetPtr;		//used to indicate which times are in ticks, and which are in msec
	int16_t playbackPtrPoles;		//used to indicate the next note to play (will be delayed by audio delay)
  int16_t playbackPtrLight;
	note currentNote;
	uint16_t lastNotePressed;	//once a button has been pressed, it isn't checked any more

	noteCalendar();
	void reset(void);
	void initNoteChecking(uint16_t in_startWindowMs, uint16_t in_endWindowMs);
	noteEvent checkForValidNote(uint32_t playTime);
	void addNote(note newNote);
	bool getNoteForPoles(uint32_t currentTimestamp);
  bool getNoteForLight(uint32_t currentTimestamp);
	void serialPrint(uint16_t start, uint16_t length);

private:
	uint16_t startWindowMs;		//used to indicated how many milliseconds before a note the button is valid
	uint16_t endWindowMs;		//and how many milliseconds after
};
#endif
