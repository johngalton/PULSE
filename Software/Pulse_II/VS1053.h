/**********************************************************
*
*    Filename     VS1053.h
*    Author       A Cowan
*    Start date   04/12/2017
*    Project      PULSE
*    Description  Class to interface with the VS1053
*                 audio codec used to play OGG files
*
*    Source       Class originally based on the VS1053
*                 library written by Adafruit; downloaded
*                 from their github on 04/12/17, and then
*                 modified significantly
*
**********************************************************/

//Original header with attribution to Adafruit:
/***************************************************
  This is a library for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#ifndef VS1053_H
#define VS1053_H

#include <Arduino.h>
#include <SPI.h> 
#include <SD.h>
#include "errorCodes.h"

#define VS1053_RST 8
#define VS1053_CS  4
#define VS1053_DCS 7
#define VS1053_REQ 2

#define VS1053_SCI_READ 0x03
#define VS1053_SCI_WRITE 0x02

#define VS1053_REG_MODE  0x00
#define VS1053_REG_STATUS 0x01
#define VS1053_REG_BASS 0x02
#define VS1053_REG_CLOCKF 0x03
#define VS1053_REG_DECODETIME 0x04
#define VS1053_REG_AUDATA 0x05
#define VS1053_REG_WRAM 0x06
#define VS1053_REG_WRAMADDR 0x07
#define VS1053_REG_HDAT0 0x08
#define VS1053_REG_HDAT1 0x09
#define VS1053_REG_VOLUME 0x0B

#define VS1053_GPIO_DDR 0xC017
#define VS1053_GPIO_IDATA 0xC018
#define VS1053_GPIO_ODATA 0xC019

#define VS1053_INT_ENABLE  0xC01A

#define VS1053_MODE_SM_DIFF 0x0001
#define VS1053_MODE_SM_LAYER12 0x0002
#define VS1053_MODE_SM_RESET 0x0004
#define VS1053_MODE_SM_CANCEL 0x0008
#define VS1053_MODE_SM_EARSPKLO 0x0010
#define VS1053_MODE_SM_TESTS 0x0020
#define VS1053_MODE_SM_STREAM 0x0040
#define VS1053_MODE_SM_SDINEW 0x0800
#define VS1053_MODE_SM_ADPCM 0x1000
#define VS1053_MODE_SM_LINE1 0x4000
#define VS1053_MODE_SM_CLKRANGE 0x8000

#define VS1053_SCI_AIADDR 0x0A
#define VS1053_SCI_AICTRL0 0x0C
#define VS1053_SCI_AICTRL1 0x0D
#define VS1053_SCI_AICTRL2 0x0E
#define VS1053_SCI_AICTRL3 0x0F

#define VS1053_DATABUFFERLEN 32

// The variable feedState is used to step through the state machine in
// chapter 9.5.1 of the datasheet. Uses the following enum:

enum feedData
{
	NOT_PLAYING,
	FEEDING_AUDIO,
	FEEDING_END,
	FEEDING_CANCEL
};

class VS1053
{
public:
	bool isInitialised;
	int32_t positionMs;
	File currentTrack;
	char filepath[40];

	VS1053();
	uint8_t initialise();
	uint8_t startPlaying(uint8_t volume);
	bool isPlaying();
	void stopPlaying(void);
	void mute(void);
	void feedBuffer(void);
	void feedBuffer_noLock(void);
	bool decodeMsec(void);

private:
	uint8_t audiobuffer[VS1053_DATABUFFERLEN];
	uint16_t endFillByte;	// defined as a 16 bit variable, only 8 bits are used
	uint16_t endBytesSent;
	uint8_t feedState;

	void reset(void);
	void softReset(void);
	void setVolume(uint8_t left, uint8_t right);
	void playData(uint8_t *buffer, uint8_t buffsiz);
	boolean readyForData(void);
	uint16_t sciRead(uint8_t addr);
	void sciWrite(uint8_t addr, uint16_t data);
	void sineTest(uint8_t n, uint16_t ms);
};

#endif // VS1053_H
