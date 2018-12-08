/***************************************
*
*    Filename     nextion.h
*    Author       A Cowan
*    Start date   27/10/2018
*    Project      PULSE
*    Description  Class representing the connected
*                 Nextion display
*
**********************************************************/

#ifndef nextion_h
#define nextion_h

#include "Arduino.h"
#include "audioLib.h"
#include "calendar.h"
#include "pulseGame.h"

#define SETUP_TITLE     "t0"
#define SETUP_STATUS0   "t1"
#define SETUP_STATUS1   "t2"
#define SETUP_STATUS2   "t3"
#define SETUP_STATUS3   "t4"
#define SETUP_STATUS4   "t5"
#define SETUP_PROGRESS  "j0"

#define LIBRARY_INDEX   "b2"
#define LIBRARY_ARTST0  "t0"
#define LIBRARY_TITLE0  "t1"
#define LIBRARY_ARTST1  "t2"
#define LIBRARY_TITLE1  "t3"
#define LIBRARY_ARTST2  "t4"
#define LIBRARY_TITLE2  "t5"
#define LIBRARY_ARTST3  "t6"
#define LIBRARY_TITLE3  "t7"
#define LIBRARY_ARTST4  "t8"
#define LIBRARY_TITLE4  "t9"

#define DETAIL_ARTIST "t0"
#define DETAIL_TITLE  "t1"
#define DETAIL_REFNUM "t3"
#define DETAIL_TNOTES "t5"
#define DETAIL_DURTON "t7"
#define DETAIL_PRSSTA "t8"

#define PLAY_ARTIST "t0"
#define PLAY_TITLE  "t1"
#define PLAY_ELAPSE "t3"
#define PLAY_DURTON "t7"
#define PLAY_PROGRS "j0"

#define PAGE_SETUP    0
#define PAGE_LIBRARY  1
#define PAGE_DETAIL   2
#define PAGE_PLAY     3

#define BUTTON_UP_ID  4
#define BUTTON_DN_ID  5
#define LIB_ITEM_0_ID 21
#define LIB_ITEM_1_ID 22
#define LIB_ITEM_2_ID 23
#define LIB_ITEM_3_ID 24
#define LIB_ITEM_4_ID 25
#define DETAIL_PLAY_ID  4
#define DETAIL_BACK_ID  5
#define PLAY_STOP_ID  5

extern pulseGame game;
extern noteCalendar noteList;

class nextion
{
  public:
    // Class variables

    // Class functions
    nextion();  //constructor
    void initalise();
    void loadPage(uint8_t pageNo);
    void setupLibrary(track* in_trackList, uint8_t numSongs);
    void populateLibrary(uint8_t pageNo);
    void populatePlayer(uint8_t trackNo);

    void updatePlayBackTime(uint32_t playbackMillis);

    void checkForInput();

    void setText(char* element, char* text);
    void setProgress(char* element, uint8_t percent);

    void flashLEDs(uint8_t ledMask, uint16_t flashTime);
    void processLEDs();
    void setLEDs(uint8_t ledMask);
    
  private:
    // Class variables
    uint8_t displayedLibraryPage;
    uint8_t maxLibraryPage;
    track* trackList;
    uint8_t selectedSong;
    uint16_t playbackSecs;

    uint16_t flashTime;
    uint32_t ledOnTime;

    // Class functions
    bool dataAvailable(char* readData);
};

#endif
