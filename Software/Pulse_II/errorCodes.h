/**********************************************************
*
*    Filename     errorCodes.h
*    Author       A Cowan
*    Start date   04/12/2017
*    Project      PULSE
*    Description  Enum of possible error codes
*                 used within the PULSE project
*
**********************************************************/

#ifndef errorCodes_h
#define errorCodes_h

enum {
	E_SUCCESS = 0,

	E_NO_MUSIC_DIR,

	E_FILENAME_TOO_LONG,

	E_UNABLE_TO_OPEN_MIDI,
	E_MIDI_MISSING_MTHD,
	E_MIDI_INVALID_FORMAT,		//we only support midi files with a format of 1 (multiple simultaneous tracks)
	E_MIDI_SMTPE_TIME_MODE,

	E_MIDI_NO_NOTES_FOUND,
	E_MIDI_NO_TEMPO_FOUND,

	E_MIDI_RUN_PARSE_FIRST,		//you must run the decode function with populateNotes and computeTimings set to false before decoding can be done
	E_MIDI_MISSING_MTRK,
	E_MIDI_UNABLE_TO_SEEK,

	E_MIDI_TEMPO_FORMAT,
	E_MIDI_INVALID_NOTE,		//outside the range of (lowestNote) >= (note) >= (lowestNote + 5)
	E_MIDI_TOO_MANY_NOTES,		//track contains more notes than calendarSize allows
	E_MIDI_OVERLAPPING_NOTES,	//simultaneous are allowed, overlapping are not
	E_MIDI_MISSING_NOTE_OFF,	//the notes file ended without turning all the notes off

	E_MIDI_TOO_FEW_BYTES,		//file ended earlier than expected

	E_UNABLE_TO_OPEN_INI,

	E_INI_MISSING_ARTIST,
	E_INI_MISSING_TITLE,
	E_INI_MISSING_DELAY,

	E_COUNTDOWN_MISSING,  //the 3,2,1,activate soundtrack is missing

	E_VS1053_INIT_ERROR,
	E_VS1053_NOT_INITIALISED,
	E_UNABLE_TO_OPEN_OGG
};

#endif
