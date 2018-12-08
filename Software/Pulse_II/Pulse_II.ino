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
#include "nextion.h"
#include "pulseGame.h"
#include <SPI.h>
#include <SD.h>

audioLib pulseAudio;

VS1053 audioCodec;

noteCalendar noteList;

pole poles;

nextion touchScreen;

pulseGame game;

#define VOLUME_COUNTDOWN 200
#define VOLUME_TRACK     180

#define SHORT_LED_FLASH_DUR 100

void setup()
{

	Serial.begin(115200);
  poles.initialise();
  delay(200);
}

void loop()
{

	/*
	Note: This file is all just test code for developing the
		  project classes and functionality. This is the bit
		  that isn't intended to be neat or at all optimised!
	*/
	
	clearTerminal();
	cursorHome();
 
  touchScreen.initalise();
  touchScreen.loadPage(PAGE_SETUP); //first command is always lost
  touchScreen.loadPage(PAGE_SETUP);
  touchScreen.setText(SETUP_TITLE, "PULSE");

	pulseAudio.initSDcard();  //populates SETUP_STATUS0 on touchscreen
	pulseAudio.scanSDcard();  //populates SETUP_STATUS1 on touchscreen
	pulseAudio.parseSDcard();  //populates SETUP_STATUS2 on touchscreen

	Serial.println();
	pulseAudio.printSongbook();
	Serial.println("\n");

  touchScreen.setupLibrary(pulseAudio.songbook, pulseAudio.numberSongs);

  touchScreen.setText(SETUP_STATUS4, "Loaded successfully");
  touchScreen.setProgress(SETUP_PROGRESS, 100);

  delay(1000);

  track dummyTrack;                         //just used for the countdown
  uint8_t blockOrder[] = {0, 1, 2, 3, 4};   //the blocks of colour used at the start
  ledBlock whiteBlock;
  whiteBlock.blockColour[0] = COLOUR_WHI;
  whiteBlock.blockColour[1] = COLOUR_WHI;
  whiteBlock.blockColour[2] = COLOUR_WHI;
  whiteBlock.blockSize = 4;

  uint16_t playTimeSecs;
  uint32_t millisOffset;  //difference between system time and playback time
  uint32_t lastSync;      //this needs to be recalculated fairly often

  uint16_t noteIndex;     //note to be played next


  //state engine driven, zero delays game play
  
  game.enterState(GAME_IDLE);
  
  while(1)
  {
    switch(game.state)
    {
      case GAME_IDLE:
      {
        if (!game.check(IDLE_INIT))
        {
          if (audioCodec.isPlaying())
          {
            Serial.println("Cancelling playback");
            audioCodec.stopPlaying();
          }
          touchScreen.loadPage(PAGE_LIBRARY);
          touchScreen.populateLibrary(0);
          game.eventDone(IDLE_INIT);
        }
        break;
      }
      case GAME_TESTING:
      {
        break;
      }
      case GAME_COUNTDOWN:
      {
        if (!game.check(COUNTDOWN_INIT))
        {
          Serial.println("Initialising Countdown");
          touchScreen.loadPage(PAGE_PLAY);
          touchScreen.populatePlayer(game.trackID);
          poles.setUpdateSpeed(25);
          dummyTrack.playCountdown(VOLUME_COUNTDOWN);
          poles.setScrollDirection(POLES123_ID, SCROLL_UP);
          randomShuffle(blockOrder);
          game.blocksFired = 0;
          game.eventDone(COUNTDOWN_INIT);
        }
        else if (!game.check(COUNTDOWN_ALL_BLOCKS_FIRED))
        {
          if (game.timeSinceLastEvent() > 400)
          {
            if (game.blocksFired < 4)
            {
              poles.addNoteBlock(0x01 << blockOrder[game.blocksFired], 4);
              touchScreen.flashLEDs(0x01 << blockOrder[game.blocksFired], 150);
              game.blocksFired++;
              game.resetEventTimer();
            }
            else //it's the fifth block
            {
              poles.addLedBlock(whiteBlock);
              touchScreen.flashLEDs(0x15, 500);
              game.eventDone(COUNTDOWN_ALL_BLOCKS_FIRED);
            }
          }
        }
        else if (!game.check(COUNTDOWN_RESETDIR))
        {
          if (game.timeSinceLastEvent() > 1400)   //allow for those blocks to reach the top before switching direction
          {
            poles.setScrollDirection(POLES123_ID, SCROLL_DOWN);
            game.eventDone(COUNTDOWN_RESETDIR);
            touchScreen.flashLEDs(0x1F, 1000);
          }
        }
        else  //we've done all the events
        {
          if (audioCodec.isPlaying() == false) //allow the countdown to finish
          {
            Serial.println("Ended Countdown");
            game.enterState(GAME_PLAYING);
          }
        }
        break;
      }//end COUNTDOWN case
      case GAME_PLAYING:
      {
        if (!game.check(PLAYING_INIT))
        {
          Serial.println("Initialising Game");
          pulseAudio.songbook[game.trackID].parseMidi();  //populate the notes calendar
          Serial.println("Starting audio");
          pulseAudio.songbook[game.trackID].playOgg(VOLUME_TRACK);
          poles.setUpdateSpeed(15);
          millisOffset = 0;
          lastSync = 0;
          noteIndex = 0;
          game.eventDone(PLAYING_INIT);
        }
        else  //we've done all the events
        {
          //Track playing
  
          ////////////// CLOCK SYNC AND TRACKING //////////////
          
          if ((millis() - lastSync) > 50)            //if we haven't synced for 50ms (3.5ms drift?)
          {
            bool decodeStat = audioCodec.decodeMsec();      //try and read the playback time to update
            if (decodeStat)
            {
              millisOffset = millis() - audioCodec.positionMs;
              lastSync = millis();
              touchScreen.updatePlayBackTime(audioCodec.positionMs);
            }
          }
  
          ////////////// NOTE LOAD AND DROP //////////////
  
          if (lastSync != 0)                    //only play notes once we've synchronised with the playback
          {
            if (noteList.getNoteForPoles(millis() - millisOffset + poles.poleDelayMs))    //check if the oldest note should have been added to the poles
            {    
             if (noteList.currentNote.duration == 1)
               poles.addNoteBlock(noteList.currentNote.event, 4);
             else
               poles.addNoteBlock(noteList.currentNote.event, noteList.currentNote.duration / poles.updateSpeedMs);
             noteIndex++;
            }

            if (noteList.getNoteForLight(millis() - millisOffset))    //check if the oldest note should have been flashed on the unit
            {    
             if (noteList.currentNote.duration == 1)
               touchScreen.flashLEDs(noteList.currentNote.event, SHORT_LED_FLASH_DUR);
             else
               touchScreen.flashLEDs(noteList.currentNote.event, noteList.currentNote.duration);
            }
          }
  
          if ((audioCodec.isPlaying() == false))// && (noteIndex == noteList.totalNotes))
          {
            Serial.println("Ended Game");
            game.enterState(GAME_FINISHED);
          }
        }
        break;
      }
      case GAME_FINISHED:
      {
        if (!game.check(FINISHED_INIT))
        {
          Serial.println("All done playing");
          game.eventDone(FINISHED_INIT);
        }
        else  //we've done all the events
        {
          game.enterState(GAME_IDLE);
        }
        break;
      }
    }//end switch statement
  
    touchScreen.processLEDs();    //check if they need to turn off (for flashing)
    touchScreen.checkForInput();
    
  }//end while loop 
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

void randomShuffle(uint8_t data[])
{
  randomSeed(millis());

  for (int i = 0; i < 10; i++)    //random shuffle
  {
    uint8_t source = random(0, 5);
    uint8_t destin = random(0, 5);
    uint8_t was_at_destin = data[destin];
    data[destin] = data[source];    //move whatever was at the source to the destination
    data[source] = was_at_destin;          //and whatever was at the destination to the source
  }
}

