/**********************************************************
*
*    Filename     calendar.cpp
*    Author       A Cowan
*    Start date   04/12/2017
*    Project      PULSE
*    Description  Class representing the notes
*                 of an entire PULSE song
*
**********************************************************/

#include "Arduino.h"
#include "calendar.h"

noteCalendar::noteCalendar()		//constructor
{
	reset();
}

/**
*	\brief Resets the calendar
*
*	Clears all notes, and resets the tempo and playback pointers
*	to their initial values
*
*	\return None
**/
void noteCalendar::reset(void)
{
	for (int i = 0; i < CALENDAR_MAX_SIZE; i++)
	{
		noteArray[i].reset();
	}
	totalNotes = 0;
	tempoSetPtr = -1;
	playbackPtr = -1;
}

/**
*	\brief Adds a note to the calendar
*
*	Adds a given note onto the end of the calendar, and increments the
*	total number of notes by one
*
*	\return None
**/
void noteCalendar::addNote(note newNote)
{
	noteArray[totalNotes].timestamp = newNote.timestamp;
	noteArray[totalNotes].event = newNote.event;
	noteArray[totalNotes].duration = newNote.duration;
	noteArray[totalNotes].configured = newNote.configured;
	totalNotes++;
}

/**
*	\brief Gets the oldest unplayed note that should have been played by now
*
*	If there are unplayed notes, it checks to see if the oldest one should
*	have been played by now. If it has, it updates 'currentNote' and increases
*	the pointer to the next oldest note.
*
*	\return Boolean; if a new note was fetched or not
**/
bool noteCalendar::getNote(uint32_t currentTimestamp)
{
	if ((playbackPtr + 2) > totalNotes)	//have we played all the notes in the calendar already?
		return false;
		
	if (currentTimestamp >= noteArray[playbackPtr + 1].timestamp)
	{
		currentNote = noteArray[playbackPtr + 1];
		playbackPtr++;
		return true;
	}
	else
	{
		return false;
	}
}

/**
*	\brief Prints the calendar to the serial terminal
*
*	Prints either a section or the entirety of the notes calendar
*	to the serial terminal. It won't print more notes than it has,
*	so a length of '0xFFFF' can be passed in to print everything
*
*	\return None
**/
void noteCalendar::serialPrint(uint16_t start, uint16_t length)
{
	if (totalNotes == 0)
	{
		Serial.println("Calendar is empty");
		return;
	}

	if (start > totalNotes)
	{
		Serial.println("Start > number of notes");
		return;
	}

	for (int pos = start; pos < (start + length); pos++)
	{
		Serial.print(pos);
		Serial.print("\t");
		Serial.print(noteArray[pos].timestamp);
		Serial.print("\t");
		Serial.print((bool)(noteArray[pos].event & 0x01));
		Serial.print(" ");
		Serial.print((bool)(noteArray[pos].event & 0x02));
		Serial.print(" ");
		Serial.print((bool)(noteArray[pos].event & 0x04));
		Serial.print(" ");
		Serial.print((bool)(noteArray[pos].event & 0x08));
		Serial.print(" ");
		Serial.print((bool)(noteArray[pos].event & 0x10));
		Serial.print("\t");
		Serial.println(noteArray[pos].duration);
    if (pos > totalNotes)
      return;
	}

}
