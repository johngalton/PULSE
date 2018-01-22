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

#include "VS1053.h"
#include <SD.h>

volatile boolean feedBufferLock = false;


#define VS1053_CONTROL_SPI_SETTING  SPISettings(250000,  MSBFIRST, SPI_MODE0)
#define VS1053_DATA_SPI_SETTING     SPISettings(8000000, MSBFIRST, SPI_MODE0)

static VS1053 *myself;

static void VS1053_IRQ(void)
{
	myself->feedBuffer();
}


void VS1053::feedBuffer(void)
{
	noInterrupts();
	// dont run twice in case interrupts collided
	// This isn't a perfect lock as it may lose one feedBuffer request if
	// an interrupt occurs before feedBufferLock is reset to false. This
	// may cause a glitch in the audio but at least it will not corrupt
	// state.
	if (feedBufferLock)
	{
		interrupts();
		return;
	}
	feedBufferLock = true;
	interrupts();
	feedBuffer_noLock();
	feedBufferLock = false;
}



void VS1053::feedBuffer_noLock(void)
{
	if (!readyForData())
	{
		return; // paused or stopped
	}

	while (readyForData())
	{
		// Read some audio data from the SD card file
		int bytesread = currentTrack.read(audiobuffer, VS1053_DATABUFFERLEN);

		if (bytesread == 0)	// must be at the end of the file, wrap it up!
		{
			currentTrack.close();
			break;
		}
		playData(audiobuffer, bytesread);
	}
}

VS1053::VS1053(void)
{
	//default constructor
	isInitialised = false;
}

uint8_t VS1053::initialise()
{
	if (isInitialised)
	{
		Serial.println("VS1053 is already initialised");
		return E_SUCCESS;
	}

	int8_t irq = digitalPinToInterrupt(VS1053_REQ);

	SPI.usingInterrupt(irq);                  //means IRQ is disabled during SPI transfer
	attachInterrupt(irq, VS1053_IRQ, CHANGE);

	myself = this;

	pinMode(VS1053_RST, OUTPUT);
	digitalWrite(VS1053_RST, LOW);
	pinMode(VS1053_CS, OUTPUT);
	digitalWrite(VS1053_CS, HIGH);
	pinMode(VS1053_DCS, OUTPUT);
	digitalWrite(VS1053_DCS, HIGH);
	pinMode(VS1053_REQ, INPUT);

	SPI.begin();
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV128);

	reset();

	if (((sciRead(VS1053_REG_STATUS) >> 4) & 0x0F) == 0x04)
	{
		isInitialised = true;
	}
	else
	{
		Serial.println("Error initialising VS1053");
		return E_VS1053_INIT_ERROR;
	}
	return E_SUCCESS;
}

void VS1053::reset()
{
	digitalWrite(VS1053_RST, LOW);
	delay(2);
	digitalWrite(VS1053_RST, HIGH);

	digitalWrite(VS1053_CS, HIGH);
	digitalWrite(VS1053_DCS, HIGH);
	delay(2);
	softReset();
	delay(10);

	sciWrite(VS1053_REG_CLOCKF, 0x6000);

	mute();
}


void VS1053::softReset(void)
{
	sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET);
	delay(10);
}


boolean VS1053::readyForData(void)
{
	return digitalRead(VS1053_REQ);
}


uint8_t VS1053::startPlaying(uint8_t volume)	//volume: 0 is quiet, 255 is full
{
	if (!isInitialised)
	{
		Serial.println("VS1053 not initialised");
		return E_VS1053_NOT_INITIALISED;
	}

	//set volume
	volume = 255 - volume;
	setVolume(volume, volume);

	// reset playback
	sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW);
	// resync
	sciWrite(VS1053_REG_WRAMADDR, 0x1e29);
	sciWrite(VS1053_REG_WRAM, 0);

  currentTrack = SD.open(filepath);

	if (!currentTrack)
	{
		Serial.println("Couldn't open track");
		return E_UNABLE_TO_OPEN_OGG;
	}

	noInterrupts();  	// don't let the IRQ get triggered while we setup

	/* The user may change the value of this register. In that case the new
	   value should be written twice to make absolutely certain that the
	   change is not overwritten by the firmware. */
	sciWrite(VS1053_REG_DECODETIME, 0x00);
	sciWrite(VS1053_REG_DECODETIME, 0x00);

	// wait till its ready for data
	while (!readyForData()) {}

	// fill it up!
	while (readyForData())
	{
		feedBuffer();
	}

	// ok going forward, we can use the IRQ
	interrupts();

	return E_SUCCESS;
}


void VS1053::stopPlaying(void)
{
	// cancel all playback
	sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_CANCEL);
	mute();

	// close the file on the card
	currentTrack.close();
}


void VS1053::playData(uint8_t *buffer, uint8_t buffsiz)
{
	SPI.beginTransaction(VS1053_DATA_SPI_SETTING);
	digitalWrite(VS1053_DCS, LOW);

	while (buffsiz--)
	{
		SPI.transfer(buffer[0]);
		buffer++;
	}

	digitalWrite(VS1053_DCS, HIGH);
	SPI.endTransaction();
}


void VS1053::mute()
{
	setVolume(0xFF, 0xFF);	//fully mute both channels (analog power down)
}


void VS1053::setVolume(uint8_t left, uint8_t right)
{
	/* SCI VOL is a volume control for the player hardware. The most significant byte of the volume register
	controls the left channel volume, the low part controls the right channel volume. The channel volume
	sets the attenuation from the maximum volume level in 0.5 dB steps. Thus, maximum volume is 0x0000
	and total silence is 0xFEFE. */

	noInterrupts();
	sciWrite(VS1053_REG_VOLUME, ((left << 8) | right));
	interrupts();
}


bool VS1053::decodeMsec()
{
	noInterrupts();
	sciWrite(VS1053_REG_WRAMADDR, 0x1e27);
	uint16_t lWord_1 = sciRead(VS1053_REG_WRAM);
	uint16_t hWord_1 = sciRead(VS1053_REG_WRAM);
	sciWrite(VS1053_REG_WRAMADDR, 0x1e27);
	uint16_t lWord_2 = sciRead(VS1053_REG_WRAM);  //read it twice to ensure it didn't change while we were crossing the byte boundary
	interrupts();

	if (lWord_1 != lWord_2) 						//the LSB changed (it ticked while we were reading)
	{
		positionMs = ((hWord_1 << 16) | lWord_1);	//this is *probably* right, but can't be certain
		return false;
	}

	if (((hWord_1 << 16) | lWord_1) == -1) //returns -1 if the position is unknown (eg it's loading)
	{
		noInterrupts();
		positionMs = (sciRead(VS1053_REG_DECODETIME) * 1000);	//best guess from the 'seconds' register
		interrupts();
		return false;
	}

	positionMs = ((hWord_1 << 16) | lWord_1);
	return true;
}


uint16_t VS1053::sciRead(uint8_t addr)
{
	uint16_t data;

	SPI.beginTransaction(VS1053_CONTROL_SPI_SETTING);
	digitalWrite(VS1053_CS, LOW);
	SPI.transfer(VS1053_SCI_READ);
	SPI.transfer(addr);
	delayMicroseconds(2);
	data = SPI.transfer(0x00);
	data <<= 8;
	data |= SPI.transfer(0x00);
	digitalWrite(VS1053_CS, HIGH);
	SPI.endTransaction();

	return data;
}


void VS1053::sciWrite(uint8_t addr, uint16_t data)
{
	SPI.beginTransaction(VS1053_CONTROL_SPI_SETTING);
	digitalWrite(VS1053_CS, LOW);
	SPI.transfer(VS1053_SCI_WRITE);
	SPI.transfer(addr);
	SPI.transfer(data >> 8);
	SPI.transfer(data & 0xFF);
	digitalWrite(VS1053_CS, HIGH);
	SPI.endTransaction();
}


void VS1053::sineTest(uint8_t n, uint16_t ms) {
	reset();

	uint16_t mode = sciRead(VS1053_REG_MODE);
	mode |= 0x0020;
	sciWrite(VS1053_REG_MODE, mode);

	while (!digitalRead(VS1053_REQ));
	//  delay(10);

	SPI.beginTransaction(VS1053_DATA_SPI_SETTING);
	digitalWrite(VS1053_DCS, LOW);
	SPI.transfer(0x53);
	SPI.transfer(0xEF);
	SPI.transfer(0x6E);
	SPI.transfer(n);
	SPI.transfer(0x00);
	SPI.transfer(0x00);
	SPI.transfer(0x00);
	SPI.transfer(0x00);
	digitalWrite(VS1053_DCS, HIGH);
	SPI.endTransaction();

	delay(ms);

	SPI.beginTransaction(VS1053_DATA_SPI_SETTING);
	digitalWrite(VS1053_DCS, LOW);
	SPI.transfer(0x45);
	SPI.transfer(0x78);
	SPI.transfer(0x69);
	SPI.transfer(0x74);
	SPI.transfer(0x00);
	SPI.transfer(0x00);
	SPI.transfer(0x00);
	SPI.transfer(0x00);
	digitalWrite(VS1053_DCS, HIGH);
	SPI.endTransaction();
}
