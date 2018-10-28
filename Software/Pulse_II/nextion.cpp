/***************************************
*
*    Filename     nextion.cpp
*    Author       A Cowan
*    Start date   27/10/2018
*    Project      PULSE
*    Description  Class representing the connected
*                 Nextion display
*
**********************************************************/

#include "Arduino.h"
#include "nextion.h"

nextion::nextion()    //default constructor
{
}

void nextion::initalise()
{
  Serial3.begin(115200);
  while(Serial3.available())
    Serial3.read(); //clear buffer
}

void nextion::loadPage(uint8_t pageNo)
{
  char terminate[4];
  terminate[0] = 0xFF;
  terminate[1] = 0xFF;
  terminate[2] = 0xFF;
  terminate[3] = '\0';

  char buff[4];
  itoa(pageNo, buff, 10);
  
  char command[14];
  strcpy(command, "page ");
  strcat(command, buff);
  strcat(command,terminate);

  Serial3.write(command);
}

void nextion::setupLibrary(track* in_trackList, uint8_t numSongs)
{
  maxLibraryPage = numSongs / 5;
  
  if ((maxLibraryPage * 5) < numSongs)  //if there was a remainder, add another page
    maxLibraryPage++;

  maxLibraryPage--; //as we're zero indexed

  trackList = in_trackList;
}

/**
*  \brief Sends the data of 5 songs to the display
*
*  The display must be showing a library page.
*  Takes the songbook, and sends the name and artist of
*  5 songs, and sends them to the display
*
* \return Nothing
**/
void nextion::populateLibrary(uint8_t pageNo)
{
  displayedLibraryPage = pageNo;

  char buff[5];
  char temp[5];
  itoa((pageNo + 1), buff, 10);         //we're zero indexed, so add 1
  itoa((maxLibraryPage + 1), temp, 10);
  strcat(buff,"/");
  strcat(buff,temp);

  setText(LIBRARY_INDEX, buff);
  setText(LIBRARY_ARTST0, trackList[pageNo * 5 + 0].artist);
  setText(LIBRARY_TITLE0, trackList[pageNo * 5 + 0].title);
  setText(LIBRARY_ARTST1, trackList[pageNo * 5 + 1].artist);
  setText(LIBRARY_TITLE1, trackList[pageNo * 5 + 1].title);
  setText(LIBRARY_ARTST2, trackList[pageNo * 5 + 2].artist);
  setText(LIBRARY_TITLE2, trackList[pageNo * 5 + 2].title);
  setText(LIBRARY_ARTST3, trackList[pageNo * 5 + 3].artist);
  setText(LIBRARY_TITLE3, trackList[pageNo * 5 + 3].title);
  setText(LIBRARY_ARTST4, trackList[pageNo * 5 + 4].artist);
  setText(LIBRARY_TITLE4, trackList[pageNo * 5 + 4].title);
}

void nextion::populatePlayer(uint8_t trackNo)
{  
  selectedSong = trackNo;
  setText(PLAY_ARTIST, trackList[selectedSong].getArtist());
  setText(PLAY_TITLE, trackList[selectedSong].getTitle());
  setText(PLAY_DURTON, trackList[selectedSong].getLength());
  setText(PLAY_ELAPSE, "");
  setProgress(PLAY_PROGRS, 0);
  playbackSecs = 0;
}

void nextion::checkForInput()
{
  char readData[10];
  if (!dataAvailable(readData))
    return;

  //otherwise there was data, and we've now read it in

  if (readData[0] == 0x1A)
  {
    //Serial.println("Variable name invalid");
  }
  else if (readData[0] == 0x65)
  {
    Serial.print("Touch event on page ");
    Serial.print((byte)readData[1]);
    Serial.print(", component ");
    Serial.println((byte)readData[2]);

    if ((readData[1] == PAGE_LIBRARY) && (readData[2] == BUTTON_UP_ID) && (displayedLibraryPage < maxLibraryPage)) //library page increase
    {
      displayedLibraryPage++;
      populateLibrary(displayedLibraryPage);
    }
    if ((readData[1] == PAGE_LIBRARY) && (readData[2] == BUTTON_DN_ID) && (displayedLibraryPage > 0)) //library page decrease
    {
      displayedLibraryPage--;
      populateLibrary(displayedLibraryPage);
    }
    if ((readData[1] == PAGE_LIBRARY) && (readData[2] >= LIB_ITEM_0_ID) && (readData[2] <= LIB_ITEM_4_ID))
    {
      selectedSong = (displayedLibraryPage * 5) + (readData[2] - LIB_ITEM_0_ID);

      uint8_t parseStatus = trackList[selectedSong].parseMidi();  //populates the notes calendar
      
      loadPage(PAGE_DETAIL);

      char songRef[3];
      itoa(selectedSong, songRef, 10);
      char totalNotes[5];
      itoa(trackList[selectedSong].numberNotes, totalNotes, 10);

      setText(DETAIL_ARTIST, trackList[selectedSong].getArtist());
      setText(DETAIL_TITLE, trackList[selectedSong].getTitle());
      setText(DETAIL_REFNUM, songRef);
      setText(DETAIL_TNOTES, totalNotes);
      setText(DETAIL_DURTON, trackList[selectedSong].getLength());
      if (parseStatus == E_SUCCESS)
        setText(DETAIL_PRSSTA, "PARSE SUCCESS");
      else
        setText(DETAIL_PRSSTA, "PARSE ERROR");
    }
    if ((readData[1] == PAGE_DETAIL) && (readData[2] == DETAIL_BACK_ID))
    {
      loadPage(PAGE_LIBRARY);
      populateLibrary(displayedLibraryPage);
    }
    if ((readData[1] == PAGE_DETAIL) && (readData[2] == DETAIL_PLAY_ID))
    {
      game.setTrack(selectedSong);
      game.enterState(GAME_COUNTDOWN);
    }
    if ((readData[1] == PAGE_PLAY) && (readData[2] == PLAY_STOP_ID))
    {
      game.enterState(GAME_IDLE);
    }
  }
  else
  {
    Serial.print("Unknown input 0x");
    Serial.println(readData[0],HEX);
  }
}


bool nextion::dataAvailable(char* readData)
{
  if (!Serial3.available())
    return false;

  char buff[15];
  uint8_t buffPtr = 0;
  uint8_t termChar = 0; //how many 0xFF characters we've received in a row

  unsigned long startTime = millis();

  while((millis() - startTime) < 2)  //carry on until we've received three 0xFF characters, or timed out
  {
    int data = Serial3.read();

    if (data != -1)   //only deal with actual data (-1 if nothing is available)
    {
      if (data == 0xFF)
      {
        termChar++;
        if (termChar == 3)  //every packet ends with three termination characters
          break;
      }
      else
      {
        termChar = 0;
        buff[buffPtr] = data;
        buffPtr++;
      }
    }
  }
  buff[buffPtr] = '\0';

  for (int i = 0; i <= buffPtr; i++)
  {
    readData[i] = buff[i];
  }
  return true;
}

void nextion::updatePlayBackTime(uint32_t playbackMillis)
{
  if ((playbackMillis / 1000) != playbackSecs)
  {
    playbackSecs = playbackMillis / 1000;
    uint8_t playbackMins = playbackSecs / 60;
    uint8_t playbackSecsRem = playbackSecs - (playbackMins * 60);

    char playbackMinSec[7];
    char playbackSecs_str[3];
    char playbackMins_str[3];
    itoa(playbackSecsRem,playbackSecs_str,10);
    itoa(playbackMins,playbackMins_str,10);

    strcpy(playbackMinSec, playbackMins_str);
    strcat(playbackMinSec, ":");
    if (playbackSecsRem < 10)
      strcat(playbackMinSec, "0");  //leading zero
    strcat(playbackMinSec, playbackSecs_str);   
    setText(PLAY_ELAPSE, playbackMinSec);

    uint32_t playProgress = (playbackSecs * 100) / trackList[selectedSong].lengthSecs;
    if (playProgress > 100)
      playProgress = 100;
    setProgress(PLAY_PROGRS, playProgress);
  }
}


void nextion::setText(char* element, char* text)
{
  char terminate[4];
  terminate[0] = 0xFF;
  terminate[1] = 0xFF;
  terminate[2] = 0xFF;
  terminate[3] = '\0';
  
  uint8_t commandLength = strlen(element) + strlen(text) + 12;
  char command[commandLength];
  strcpy(command, element);
  strcat(command, ".txt=\"");
  strcat(command, text);
  strcat(command, "\"");
  strcat(command,terminate);

  Serial3.write(command);
}

void nextion::setProgress(char* element, uint8_t percent)
{
  if (percent > 100)
    percent = 100;
    
  char terminate[4];
  terminate[0] = 0xFF;
  terminate[1] = 0xFF;
  terminate[2] = 0xFF;
  terminate[3] = '\0';
  
  char buff[4];
  itoa(percent, buff, 10);
  
  char command[15];
  strcpy(command, element);
  strcat(command, ".val=");
  strcat(command, buff);
  strcat(command,terminate);

  Serial3.write(command);
}


