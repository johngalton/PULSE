/**********************************************************
*
*    Filename     Pulse_II.ino
*    Author       A Cowan
*    Start date   30/11/2017
*    Project      PULSE
*    Description  Top level arduino project for the
*                 second version of PULSE. This is
*                 designed to run on an Arduino Due.
*
**********************************************************/

#include "Arduino.h"
#include "calendar.h"
#include "audioLib.h"
#include "track.h"
#include "pole.h"
#include "note.h"
#include "VS1053.h"
#include <SPI.h>
#include <SD.h>

audioLib pulseAudio;

VS1053 audioCodec;

noteCalendar noteList;

pole poles;

void setup()
{

	Serial.begin(115200);
  poles.initialise();
}

void loop()
{

/*while(1)
{
  poles.addNoteBlock(0x10,2);
  delay(300);
  poles.addNoteBlock(0x08,2);
  delay(300);
  poles.addNoteBlock(0x04,2);
  delay(300);
  poles.addNoteBlock(0x02,2);
  delay(300);
  poles.addNoteBlock(0x01,2);
  delay(300);
}*/

  
	/*
	Note: This file is all just test code for developing the
		  project classes and functionality. This is the bit
		  that isn't intended to be neat or at all optimised!
	*/
	
	clearTerminal();
	cursorHome();

	pulseAudio.initSDcard();
	pulseAudio.scanSDcard();
	pulseAudio.parseSDcard();

	Serial.println();

	pulseAudio.printSongbook();

	Serial.println("\n");
/*
	while (Serial.read() > 0) {}	//ensure serial buffer is cleared

	Serial.println("Please enter song number to play");

	while (Serial.peek() < '0')				//wait for serial input
    Serial.read();                  //discard anything that's not an ascii number

  delay(200);
  Serial.setTimeout(500);
	int input = Serial.parseInt();

	Serial.print("\nSong ");
	Serial.print(input);
	Serial.println(" selected");
	delay(1000);*/

  int input = 22;
  
	if (pulseAudio.songbook[input].parseMidi() != E_SUCCESS)
	{
		Serial.println("Unable to parse");
		while(1) { }
	}  

  poles.setUpdateSpeed(25);
  
	track dummyTrack;
	dummyTrack.playCountdown(225);

  poles.setScrollDirection(POLES123_ID, SCROLL_UP);

  uint8_t randomOrder[] = {0, 1, 2, 3, 4};

  randomSeed(millis());

  for (int i = 0; i < 10; i++)    //random shuffle
  {
    uint8_t source = random(0, 5);
    uint8_t destin = random(0, 5);
    uint8_t was_at_destin = randomOrder[destin];
    randomOrder[destin] = randomOrder[source];    //move whatever was at the source to the destination
    randomOrder[source] = was_at_destin;          //and whatever was at the destination to the source
  }

  delay(400);  //3, 2, 1, ACTIVATE

  for (int i = 0; i < 5; i++)
  {
    poles.addNoteBlock(0x01 << randomOrder[i], 4);
    delay(400);
  }

  ledBlock whiteBlock;
  whiteBlock.blockColour[0] = COLOUR_WHI;
  whiteBlock.blockColour[1] = COLOUR_WHI;
  whiteBlock.blockColour[2] = COLOUR_WHI;
  whiteBlock.blockSize = 4;

  poles.addLedBlock(whiteBlock);

  delay(1400);  //allow for those blocks to reach the top before switching direction

  poles.setScrollDirection(POLES123_ID, SCROLL_DOWN);
  
	while (audioCodec.isPlaying()) {}

	pulseAudio.songbook[input].playOgg(200);

	uint16_t index = 0;
 
	uint32_t millisOffset = 0;
	uint32_t lastSync = 0;

  poles.setUpdateSpeed(15);
  uint32_t poleOffset = poles.poleDelayMs;

	while ((index < noteList.totalNotes) && (audioCodec.isPlaying()))
	{
  //we need to resync the clock to the playback timer regularly. Typical drift is 70mS per second
		if ((millis() - lastSync) > 50)						//if we haven't synced for 50ms (3.5ms drift?)
		{
			bool decodeStat = audioCodec.decodeMsec();			//try and read the playback time to update
			if (decodeStat)
			{
				millisOffset = millis() - audioCodec.positionMs;
				lastSync = millis();
			}
		}

		if (lastSync != 0)										//only play notes if we've synchronised with the playback
		{
			if (noteList.getNote(millis() - millisOffset + poleOffset))		//check if the oldest note should have been played or not
			{
				/*cursorHome();
				printButtonState(noteList.currentNote.event);

				if (noteList.currentNote.duration < 10)
					delay(110);
				else
					delay(noteList.currentNote.duration);

				cursorHome();
				printButtonState(0x00);*/

       if (noteList.currentNote.duration == 1)
        poles.addNoteBlock(noteList.currentNote.event, 4);
       else
        poles.addNoteBlock(noteList.currentNote.event, noteList.currentNote.duration / poles.updateSpeedMs);

				index++;
			}
		}
		
		if (Serial.read() == 'c')
			audioCodec.stopPlaying();
	}
	Serial.println();
}

void clearTerminal(void)
{
	Serial.write(27);       // ESC command
	Serial.print("[2J");    // clear screen command
	cursorHome();
}

void cursorHome(void)
{
	Serial.write(27);
	Serial.print("[H");     // cursor to home command
}

void printButtonState(uint8_t state)
{
	Serial.print(" ");
	for (uint8_t button = 0; button < 5; button++)
	{
		if (state & (0x01 << button))
				Serial.print("O");
			else
				Serial.print(".");
		Serial.print(" ");
	}
}
