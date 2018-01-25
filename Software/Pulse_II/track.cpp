/**********************************************************
*
*    Filename     track.cpp
*    Author       A Cowan
*    Start date   30/11/2017
*    Project      PULSE
*    Description  Class representing one of the
*                 possible tracks that could be
*                 selected for a PULSE game
*
**********************************************************/

#include "Arduino.h"
#include "track.h"

track::track()    //constructor
{
	parseStatus = PARSE_CLEARED;
	songPath[0] = 0;  //null pointer
	artist = NULL;    //initialise to null
	title = NULL;
	presetLowestNote = 0;
}

/**
*	\brief Play OGG file
*
*	Targets the audio codec to the required track, and initialises playback
*
*	\return An integer representing status
**/
uint8_t track::playOgg(void)					//determines the length, and validity
{
	if (!audioCodec.isInitialised)
		audioCodec.initialise();

	strcpy(audioCodec.filepath, songPath);
	strcat(audioCodec.filepath, "GUITAR.OGG");

	return audioCodec.startPlaying(200);	//volume is 0 to 255
}

/**
*	\brief Parse INI file
*
*	Scans a song's .INI file. Decodes artist, title and delay information, as
*	well as top_note parameter (which is actually the lowest note) if present
*
*	\return An integer representing status
**/
uint8_t track::parseIni(void)					//reads associated parameters
{
	char filename[24];
	strcpy(filename, songPath);
	strcat(filename, "SONG.INI");

	File iniFile = SD.open(filename);

	if (!iniFile)
	{
		Serial.print("Unable to open ");
		Serial.println(filename);
		return E_UNABLE_TO_OPEN_INI;
	}

	//Ini file has been opened - now we can read it!

	parseStatus |= PARSE_INI_FOUND;

	int ptr = 0;
	char lineBuffer[INI_MAX_LINE];

	while (true)
	{
		char c = iniFile.read();

		{
			//process a line when we reach a carriage return, line feed, EOF, or the buffer is full
			if ((c != 13) && (c != 10) && (c != 255) && (ptr < (INI_MAX_LINE - 2)))	//not yet reached a carriage return, so put it into the line buffer
			{
				lineBuffer[ptr] = c;
				ptr++;
			}
			else        //completed line decoded, so decode whatever is in the line buffer
			{
				lineBuffer[ptr] = 0;  //null terminate string (we've already incremented one)
				char* strPtr;
				strPtr = strstr(lineBuffer, "artist");

				if (strPtr != NULL)
				{
					//found artist in this line
					uint8_t totalLength = strlen(strPtr);
					uint8_t startLength = strlen("artist") + 3;

					artist = (char *)malloc(totalLength - startLength + 1);

					strcpy(artist, (strPtr + startLength));

					parseStatus |= PARSE_ART_FOUND;
				}

				strPtr = strstr(lineBuffer, "name");
				if (strPtr != NULL)
				{
					//found artist in this line
					uint8_t totalLength = strlen(strPtr);
					uint8_t startLength = strlen("name") + 3;

					title = (char *)malloc(totalLength - startLength + 1);

					strcpy(title, (strPtr + startLength));

					parseStatus |= PARSE_TTL_FOUND;
				}

				strPtr = strstr(lineBuffer, "delay");
				if (strPtr != NULL)
				{
					//found artist in this line
					uint8_t totalLength = strlen(strPtr);
					uint8_t startLength = strlen("delay") + 3;

					char strDelay[8];
					strcpy(strDelay, (strPtr + startLength));
					midi_offset = atoi(strDelay);

					parseStatus |= PARSE_DEL_FOUND;
				}

				strPtr = strstr(lineBuffer, "top_note");
				if (strPtr != NULL)
				{
					//found artist in this line
					uint8_t totalLength = strlen(strPtr);
					uint8_t startLength = strlen("top_note") + 3;

					char strTop[8];
					strcpy(strTop, (strPtr + startLength));
					presetLowestNote = atoi(strTop);          
					parseStatus |= PARSE_TOP_FOUND;
				}

				ptr = 0;	//next line restarts at the start of the line buffer

        if (c == 255)
        {
          break;  //end of file
        }
			}
		}
	}

	if ((parseStatus & PARSE_ART_FOUND) == 0x00)
	{
		Serial.println("Artist not found");
		return E_INI_MISSING_ARTIST;
	}
	if ((parseStatus & PARSE_TTL_FOUND) == 0x00)
	{
		Serial.println("Title not found");
		return E_INI_MISSING_TITLE;
	}
	if ((parseStatus & PARSE_DEL_FOUND) == 0x00)
	{
		Serial.println("Delay not found");
		return E_INI_MISSING_DELAY;
	}
	return E_SUCCESS;
}

/**
*	\brief Parse a midi file
*
*	Reads a midi file, and decodes the information. Puts the notes from the
*	first note track into the noteList, then compute timestamps for all of
*	these using the first tempo track
*
*	\return An integer representing status
**/
uint8_t track::parseMidi(void)
{
	char filename[30];
	if ((strlen(songPath) + 9) > 30)
		return E_FILENAME_TOO_LONG;
	strcpy(filename, songPath);
	strcat(filename, "NOTES.MID");

	uint8_t result;
	midi midiFile(filename);               //loads the midi file
	if (result != E_SUCCESS) return result;
	result = midiFile.decodeHeader();      //decodes the file's header, works out the number of tracks
	if (result != E_SUCCESS) return result;
	result = midiFile.scanTracks();        //examines what is in all of the tracks, and checks the file's integrity
	if (result != E_SUCCESS) return result;
	result = midiFile.populateCalendar(presetLowestNote);  //finds the first track containing notes, and puts these notes into the calendar (timestamped by ticks)
	if (result != E_SUCCESS) return result;
	result = midiFile.computeTimings(midi_offset);    //finds the first track containing tempo information, and converts the calendar into milliseconds. Applies a fixed start offset to all notes
	return result;
}

/**
*	\brief Returns the artist name
*
*	\return A pointer to the name string
**/
char* track::getArtist(void)
{
	if (parseStatus & PARSE_ART_FOUND)
		return artist;
	else
		return NULL;
}

/**
*	\brief Returns the song title
*
*	\return A pointer to the name string
**/
char* track::getTitle(void)
{
	if (parseStatus & PARSE_TTL_FOUND)
		return title;
	else
		return NULL;
}

/**
*	\brief Returns if the ini file has been loaded or not
*
*	\return A boolean; true means loaded
**/
bool track::iniLoaded(void)
{
	return (parseStatus & PARSE_INI_FOUND);
}


