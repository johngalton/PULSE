/***************************************
*
*    Filename     pulseGame.h
*    Author       A Cowan
*    Start date   28/10/2018
*    Project      PULSE
*    Description  Class representing the current game state
*
**********************************************************/

#ifndef pulseGame_h
#define pulseGame_h

#include "Arduino.h"

#define IDLE_INIT       0x0001

#define COUNTDOWN_INIT              0x0001
#define COUNTDOWN_ALL_BLOCKS_FIRED  0x0002
#define COUNTDOWN_RESETDIR          0x0004

#define PLAYING_INIT    0x0001

#define FINISHED_INIT   0x0001

enum gameStates
{
	GAME_IDLE,
  GAME_TESTING,
	GAME_COUNTDOWN,
	GAME_PLAYING,
	GAME_FINISHED
};


class pulseGame
{
  public:
    // Class variables
    uint8_t state;
    uint8_t trackID;

    uint8_t blocksFired;  //only used for the countdown state

    // Class functions
    pulseGame();  //constructor
    void enterState(uint8_t newState);
    void setTrack(uint8_t in_trackID);
    uint32_t timeInState();
    bool check(uint16_t checkEvent);
    uint32_t timeSinceLastEvent();
    void resetEventTimer();
    void eventDone(uint16_t newEvent);
    
  private:
    // Class variables
    uint32_t stateStartTime;
    uint32_t lastEventTime;
    uint16_t stateEventLog; //used for flagging when we'vbe done certain things

    // Class functions
};

#endif
