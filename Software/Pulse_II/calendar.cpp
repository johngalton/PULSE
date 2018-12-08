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
	playbackPtrPoles = -1;
  playbackPtrLight = -1;
}

/**
*	\brief Initialises the variables used for aligning events with notes
*
*	Initilises the window durations (in ms), and resets the last pressed note
*
*	\return None
**/
void noteCalendar::initNoteChecking(uint16_t in_startWindowMs, uint16_t in_endWindowMs)
{
	  startWindowMs = in_startWindowMs;
	  endWindowMs = in_endWindowMs;
	  lastNotePressed = 0;
}

/**
*	\brief Checks to see if a valid note is happening at this time
*
*	Takes a time in milliseconds. Checks whether a we're in the window
*	of a valid note, and returns this information
*
*	\return The note validity/information
**/
noteEvent noteCalendar::checkForValidNote(uint32_t playTime)
{
	noteEvent thisNote;
	
	// Look through the calender from lastNotePressed (we can ignore
	// anything before that as it's in the past), and up to totalNotes.
	
	for (int i = lastNotePressed; i < totalNotes; i++)
	{
		if ((playTime > (noteArray[i].timestamp - startWindowMs)) && (playTime < (noteArray[i].timestamp + endWindowMs)))
		{
			// We've found a note valid when the event happened. Return the details
			lastNotePressed = i;
			thisNote.noteNumber = i;
			thisNote.noteDetail.timestamp = noteArray[i].timestamp;
			thisNote.noteDetail.event = noteArray[i].event;
			thisNote.noteDetail.duration = noteArray[i].duration;
			thisNote.isValid = true;
			return thisNote;
		}
		
		// If the position we are checking in the calendar is now ahead of playback, no point in searching any further
		if (((noteArray[i].timestamp + endWindowMs)) > playTime)
		{
			break;
		}
	}
	
	//no note found
	thisNote.noteNumber = 0;
	thisNote.noteDetail.timestamp = 0;
	thisNote.noteDetail.event = 0x00;
	thisNote.noteDetail.duration = 0;
	thisNote.isValid = false;
	return thisNote;
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
*	the pointer to the next oldest note. There are two pointers tracked by the
* class; one for adding notes to poles (which includes the delay), and one
* for flashing the lights on the box
*
*	\return Boolean; if a new note was fetched or not
**/
bool noteCalendar::getNoteForPoles(uint32_t currentTimestamp)
{
	if ((playbackPtrPoles + 2) > totalNotes)	//have we played all the notes in the calendar already?
		return false;
		
	if (currentTimestamp >= noteArray[playbackPtrPoles + 1].timestamp)
	{
		currentNote = noteArray[playbackPtrPoles + 1];
		playbackPtrPoles++;
		return true;
	}
	else
	{
		return false;
	}
}

/**
*  \brief Gets the oldest unplayed note that should have been played by now
*
* If there are unplayed notes, it checks to see if the oldest one should
* have been played by now. If it has, it updates 'currentNote' and increases
* the pointer to the next oldest note. There are two pointers tracked by the
* class; one for adding notes to poles (which includes the delay), and one
* for flashing the lights on the box
*
* \return Boolean; if a new note was fetched or not
**/
bool noteCalendar::getNoteForLight(uint32_t currentTimestamp)
{
  if ((playbackPtrLight + 2) > totalNotes)  //have we played all the notes in the calendar already?
    return false;
    
  if (currentTimestamp >= noteArray[playbackPtrLight + 1].timestamp)
  {
    currentNote = noteArray[playbackPtrLight + 1];
    playbackPtrLight++;
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
