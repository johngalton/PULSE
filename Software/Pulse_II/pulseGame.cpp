/***************************************
*
*    Filename     pulseGame.cpp
*    Author       A Cowan
*    Start date   28/10/2018
*    Project      PULSE
*    Description  Class representing the current game state
*
**********************************************************/

#include "Arduino.h"
#include "pulseGame.h"

pulseGame::pulseGame()    //default constructor
{
  state = GAME_IDLE;
  stateEventLog = 0x00; //clear event log
}

void pulseGame::enterState(uint8_t newState)
{
  state = newState;
  stateStartTime = millis();
  lastEventTime = millis();
  stateEventLog = 0x00; //clear event log
}

void pulseGame::setTrack(uint8_t in_trackID)
{
  trackID = in_trackID;
}

bool pulseGame::check(uint16_t checkEvent)
{
  if (stateEventLog & checkEvent)
    return true;
  else
    return false;
}

void pulseGame::eventDone(uint16_t newEvent)
{
  stateEventLog |= newEvent;
  lastEventTime = millis();
}

uint32_t pulseGame::timeInState()
{
  return (millis() - stateStartTime);
}

uint32_t pulseGame::timeSinceLastEvent()
{
  return (millis() - lastEventTime);
}

void pulseGame::resetEventTimer()
{
  lastEventTime = millis();
}


