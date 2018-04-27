/**********************************************************
*
*    Filename     audioLib.cpp
*    Author       A Cowan
*    Start date   30/11/2017
*    Project      PULSE
*    Description  Collection of library functions
*                 for handling the song book
*
**********************************************************/

#include "Arduino.h"
#include "audioLib.h"

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

#define SD_CARD_CS 10

audioLib::audioLib()		//constructor
{

}

/**
*	\brief SD card initialization
*
*	Establishes comms with the SD card, checking the type and for
*	correct formatting/blocks. Then opens filesystem.
*
*	\return An integer representing status
**/
int audioLib::initSDcard(void)
{
  while(1)
  {
  	if (!SD.begin(SD_CARD_CS))
  	{
  		Serial.println("SD Card initialization failed.");
  		delay(1000);
  	}
   else
   {
    return 1;
   }
  }
}

/**
*	\brief SD card scan
*
*	Scans the SD card, creates an array of songs with size equal to the number of
*	directories within /music/. This allocates memory (via 'new', so only ever runs
*	once (uses up a chunk of RAM).
*
*	\return An integer representing status
**/
int audioLib::scanSDcard(void)
{
	File musicFolder = SD.open("/Music");

	if (!musicFolder)
	{
		Serial.println("Couldn't open /Music directory");
		return E_NO_MUSIC_DIR;
	}

	int subFolderCount = 0;

	while (true)
	{
		File songFolder = musicFolder.openNextFile();

		if (!songFolder)
		{
			break;		// no more files or folders
		}

		if (songFolder.isDirectory())	//only print folders
		{
			subFolderCount++;
		}

		songFolder.close();
	}

	musicFolder.close();

	Serial.print("Scan complete; ");
	Serial.print(subFolderCount);
	Serial.println(" subfolders found in /music");

	songbook = new track[subFolderCount];

	return E_SUCCESS;
}

/**
*	\brief SD card parse
*
*	Goes through the SD card's /Music folder, populating the array created by
*	'scanSDcard' with song folder names and decoded INI file information
*
*	\return An integer representing status
**/
int audioLib::parseSDcard(void)
{
	File musicFolder = SD.open("/Music");

	if (!musicFolder)
	{
		Serial.println("Couldn't open /Music directory");
		return E_NO_MUSIC_DIR;
	}

	int songNumber = 0;

	while (true)
	{
		File songFolder = musicFolder.openNextFile();

		if (!songFolder)
		{
			break;    // no more files or folders
		}

		if (songFolder.isDirectory()) //a folder with music in it
		{
			songbook[songNumber].track_ID = songNumber;        //also position in songbook array
			char songPath[24];
			strcpy(songPath, musicFolder.name());
			strcat(songPath, "/");
			strcat(songPath, songFolder.name());
			strcat(songPath, "/");
			strcpy(songbook[songNumber].songPath, songPath);
			songbook[songNumber].parseStatus |= 0x01;   //found a folder for this track

			while (true)
			{
				File songFile = songFolder.openNextFile();

				if (!songFile)  //if there are no more files in this directory
					break;

				if (strcmp(songFile.name(), "SONG.INI") == 0)
				{
					songbook[songNumber].parseIni();  //read in all the constants
				}
				if (strcmp(songFile.name(), "NOTES.MID") == 0)
				{
					songbook[songNumber].parseStatus |= 0x40;              //mark that it is present
		  //          songbook[songNumber].parseMid();  //check it is present
				}
				if (strcmp(songFile.name(), "GUITAR.OGG") == 0)
				{
					songbook[songNumber].parseStatus |= 0x80;              //mark that it is present
		  //          songbook[songNumber].parseOgg();
				}
				// else it was another file time (eg .EOF?)
			}
			songNumber++; //we've reached the end of a song folder, so move on to the next
		}

		songFolder.close();
	}
	musicFolder.close();

	numberSongs = songNumber;
	return E_SUCCESS;
}

/**
*	\brief Print list of decoded songs
*
*	For each of the songs decoded, prints them out to the serial terminal.
*	Also prints any issues (such as a missing INI or OGG file)
*
*	\return An integer representing status
**/
int audioLib::printSongbook(void)
{
	Serial.println("\nSongbook:");
	for (int i = 0; i < numberSongs; i++)
	{
		Serial.print(i);
		Serial.print(": ");
		if (songbook[i].iniLoaded())
		{
			Serial.print(songbook[i].getTitle());
			Serial.print(" by ");
			Serial.print(songbook[i].getArtist());

			if ((songbook[i].parseStatus & 0xDF) != 0xDF) //don't bother reporting a missing top_note as a parse error
			{
				Serial.println("  (partial parse error)");
			}
			else
			{
				Serial.println();
			}
		}
		else
		{
			Serial.println("Missing .ini file");
		}
	}
	return E_SUCCESS;
}

