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
	// don't run twice in case interrupts collided
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
		// The DREQ line goes high if it can accept at least 32 bytes of data. This means
		// if we are here, the buffer will happily accept a chunk of 32 bytes of data. The
		// variable feedState is used to determine what kind of data is required.

		// How to play an audio file (see VS1053 datasheet, chapter 9.5.1)
		//     1) Send an audio file
		//     2) Once this finishes, send at least 2052 bytes of endFillByte
		//     3) Set SCI_MODE bit SM_CANCEL
		//     4) Send at least 32 bytes of endFillByte
		//     5) If the cancel hasn't happened within 2048 endFillBytes, do a soft reset

		switch (feedState)
		{
			case NOT_PLAYING:
					{
						return;
					}
					break;
			case FEEDING_AUDIO:
					{
						// Read some audio data from the SD card file
						int bytesread = currentTrack.read(audiobuffer, VS1053_DATABUFFERLEN);
						
						// See if we're at the end of the audio file yet
						if (bytesread < VS1053_DATABUFFERLEN)
						{
							while(bytesread < VS1053_DATABUFFERLEN)
							{
								audiobuffer[bytesread] = (endFillByte & 0xFF);
								bytesread++;
								endBytesSent++;
							}
							currentTrack.close();	//we're at the end of the audio file
							feedState = FEEDING_END;
						}
						playData(audiobuffer, bytesread);
					}
					break;
			case FEEDING_END:
					{
						if (endBytesSent < 2052)
						{
							for (int i = 0; i < 32; i++)
							{
								audiobuffer[i] = (endFillByte & 0xFF);
							}
							endBytesSent += 32;
							playData(audiobuffer, 32);
						}
						else
						{
							// We've sent enough end bytes - set cancel
							feedState = FEEDING_CANCEL;
							uint16_t mode = sciRead(VS1053_REG_MODE);
              sciWrite(VS1053_REG_MODE, (mode | VS1053_MODE_SM_CANCEL));
							endBytesSent = 0;
						}
					}
					break;
			case FEEDING_CANCEL:
					{
						// Send 32 more bytes of endFillByte, which should complete the cancel
						for (int i = 0; i < 32; i++)
						{
							audiobuffer[i] = (endFillByte & 0xFF);
						}
						endBytesSent += 32;
						playData(audiobuffer, 32);
						
						// When we read back it is no longer still cancelling, we can stop sending blank data
						uint16_t mode = sciRead(VS1053_REG_MODE);
						if ((mode & VS1053_MODE_SM_CANCEL) == 0x00)
						{
							//Playback and completion of playback complete!
							feedState = NOT_PLAYING;
							return;
						}

						// Otherwise it needs more endFillByte
						
						if (endBytesSent > 2048)
						{
							softReset();
							
							// VS1053 datasheet:
							// "If SM CANCEL hasn’t cleared after sending 2048 bytes, do a software reset (this should be extremely rare)."
							
							Serial.println("Ahh! Extremely rare thing went wrong");
						}
					}
					break;
		}
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

  initI2S();

	feedState = NOT_PLAYING;

	return E_SUCCESS;
}

uint8_t VS1053::initI2S()
{
  //initases the I2S output by writing to the I2S_CONFIG register
    // bit 3: MCLK_ENA = 0
    // bit 2: I2S_ENA = 1
    // bits 10: I2S rate = 10 = 192
    //                     01 = 96
    //                     00 = 48

  sciWrite(VS1053_REG_WRAMADDR, VS1053_GPIO_DDR);
  sciWrite(VS1053_REG_WRAM, 0xF3);

  sciWrite(VS1053_REG_WRAMADDR, VS1053_I2S_CONFIG); //we want to write to the I2S_CONFIG register
  //sciWrite(VS1053_REG_WRAM, 0x04);  //48kHz output
  //sciWrite(VS1053_REG_WRAM, 0x05);  //96kHz output
  sciWrite(VS1053_REG_WRAM, 0x06);  //192kHz output
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

	// as we're starting, we haven't send any end bytes
	endBytesSent = 0;
	feedState = FEEDING_AUDIO;

	// wait till its ready for data
	while (!readyForData()) {}

	// fill it up!
	while (readyForData())
	{
		feedBuffer();
	}
	
	// read back the endFillByte
	sciWrite(VS1053_REG_WRAMADDR, 0x1e06);
	endFillByte = sciRead(VS1053_REG_WRAM);

	// ok going forward, we can use the IRQ
	interrupts();

	return E_SUCCESS;
}

bool VS1053::isPlaying(void)
{
	if (feedState == NOT_PLAYING)
		return false;
	else
		return true;
}


void VS1053::stopPlaying(void)	// cancel all playback
{
	// Feed endFillByte when the IRQ fires
	feedState = FEEDING_END;
	
	// Close the file on the SD card
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
