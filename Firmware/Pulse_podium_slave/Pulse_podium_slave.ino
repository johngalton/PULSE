#include <Adafruit_NeoPixel.h>

#define MSTR_LED_GRE 3
#define MSTR_LED_YEL 4
#define SLVE_LED_GRE 2
#define SLVE_LED_YEL 5

#define BUT_SW 7
#define BUT_LED 6

#define DEBUG_BUT 10

enum receiveResults
{
  RESULT_DATA_RECEIVED,
  RESULT_TIMEOUT
};

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, BUT_LED, NEO_GRB + NEO_KHZ800);

byte targetRed = 0;
byte targetGre = 0;
byte targetBlu = 0;

byte actualRed = 0;
byte actualGre = 0;
byte actualBlu = 0;

bool testModeActive = 0;

unsigned long lastChangeTime = 0; // used for debouncing button presses
unsigned long lastRxPacket = 0;   // used for the RJ45 jack status indicators
unsigned long colourUpdate = 0;   // used for the colour fading timings

byte buttonAddress = 0;           // 0 is undefined, 1-5 are podium numbers

bool buttonState[5];              // used for remembering what state buttons further up the chain are in

bool myLastButState = true;;

void setup()
{
	pinMode(MSTR_LED_GRE, OUTPUT);
	pinMode(MSTR_LED_YEL, OUTPUT);
	pinMode(SLVE_LED_GRE, OUTPUT);
	pinMode(SLVE_LED_YEL, OUTPUT);

	pinMode(BUT_SW, INPUT);
	pinMode(BUT_LED, OUTPUT);

	pinMode(DEBUG_BUT, INPUT);
	digitalWrite(DEBUG_BUT, HIGH);

	Serial.begin(9600);

	pixel.begin(); // This initializes the NeoPixel library.

	pixel.setPixelColor(0, pixel.Color(0, 0, 0));
	pixel.show();
	
	for (int i = 0; i < 5; i++)
		buttonState[i] = false;    //assume all buttons are unpressed
}

void loop()
{
	if (!digitalRead(DEBUG_BUT))
	{
		pixel.setPixelColor(0, pixel.Color(255, 0, 0));
		pixel.show();
		delay(1000);
		pixel.setPixelColor(0, pixel.Color(0, 255, 0));
		pixel.show();
		delay(1000);
		pixel.setPixelColor(0, pixel.Color(0, 0, 255));
		pixel.show();
		delay(1000);
		pixel.setPixelColor(0, pixel.Color(0, 0, 0));
		pixel.show();
		actualRed = 0;  //update what they've been left as
		actualGre = 0;
		actualBlu = 0;
	}

	if (testModeActive)
	{
		if (!digitalRead(BUT_SW))
		{
			  actualRed = 255;
			  actualGre = 0;
		}
		else
		{
			  actualRed = 0;
			  actualGre = 255;
		}
		actualBlu = 0;
		pixel.setPixelColor(0, pixel.Color(actualRed, actualGre, actualBlu));
		pixel.show();
		delay(20);    //50Hz update in test mode only
	}

	byte c = Serial.read();

	if (c != -1)  //if it's an actual packet in the buffer
	{
		byte newColour = 255;      //assume invalid
		lastRxPacket = millis();

		unsigned long packetStartTime = millis();

		if (c == 0xF0)        // first in chain, message from master
		{
			if (waitForDataOrTimeout(5) == RESULT_DATA_RECEIVED)
			{
				buttonAddress = 1;
				newColour = Serial.read();    // use colour 1
				Serial.write(0xF1);
				Serial.write(Serial.read());  // send on colour 2
				Serial.write(Serial.read());  // send on colour 3
				Serial.write(Serial.read());  // send on colour 4
				Serial.write(Serial.read());  // send on colour 5
				testModeActive = false;
			}
		}
		else if (c == 0xF1)   // second in chain, message from button 1
		{
			if (waitForDataOrTimeout(4) == RESULT_DATA_RECEIVED)
			{
				buttonAddress = 2;
				newColour = Serial.read();    // use colour 2
				Serial.write(0xF2);
				Serial.write(Serial.read());  // send on colour 3
				Serial.write(Serial.read());  // send on colour 4
				Serial.write(Serial.read());  // send on colour 5
				testModeActive = false;
			}
		}
		else if (c == 0xF2)   // third in chain, message from button 2
		{
			if (waitForDataOrTimeout(3) == RESULT_DATA_RECEIVED)
			{
				buttonAddress = 3;
				newColour = Serial.read();    // use colour 3
				Serial.write(0xF3);
				Serial.write(Serial.read());  // send on colour 4
				Serial.write(Serial.read());  // send on colour 5
				testModeActive = false;
			}
		}
		else if (c == 0xF3)   // etc
		{
			if (waitForDataOrTimeout(2) == RESULT_DATA_RECEIVED)
			{
				buttonAddress = 4;
				newColour = Serial.read();    // use colour 4
				Serial.write(0xF4);
				Serial.write(Serial.read());  // send on colour 5
				testModeActive = false;
			}
		}
		else if (c == 0xF4)
		{
			if (waitForDataOrTimeout(1) == RESULT_DATA_RECEIVED)
			{
				buttonAddress = 5;
				newColour = Serial.read();    // use colour 5
				Serial.print("OK");           // send chain confirmation
				testModeActive = false;
			}
		}
		else if ((c & 0xE0) == 0xC0)          // a button further up the chain is sending data command (top 3 bits are 110)
		{
			storeButtonStatus(buttonAddress, c);  // remember what all upstream buttons are currently (we'll need to know that later)

			if (!digitalRead(BUT_SW) && (buttonAddress != 0))
				Serial.write(c | (1 << (5 - buttonAddress)));
			else
				Serial.write(c);
		}
		else if (c == 0xFB)
		{
			testModeActive = true;
			Serial.write(0xFB);
		}
		else if (c == 0xFC)
		{
			testModeActive = false;
			Serial.write(0xFC);
		}
		//else serial buffer had something other than a command, so we discard it and move on

		
		/* If the button changes state, send an event message out. */
		if (digitalRead(BUT_SW) != myLastButState)
		{
			// Only broadcast the new state if we haven't just done so
			if ((millis() - lastChangeTime) > 20)
			{
				lastChangeTime = millis();
				myLastButState = !myLastButState;

				if (buttonAddress > 0)
				{
					uint8_t outData = 0xC0;

					// We need to populate the data from buttons further up the chain, based
					// on what we've heard earlier (via data passing through us)
					for (int i = 0; i < 5; i++)
					{
						if (buttonAddress > (i+1))
							outData = (outData | (buttonState[i] << (5-i)));
					}

					// Also populate your own data
					outData = outData | (1 << (5 - buttonAddress));
					
					// Add the header and send!
					Serial.write(outData | 0xC0);
				}
			}
		}

		if (newColour == 0)
		  setNewTargetColour(0, 0, 0);
		else if (newColour == 1)
		  setNewTargetColour(255, 0, 0);
		else if (newColour == 2)
		  setNewTargetColour(255, 255, 0);
		else if (newColour == 3)
		  setNewTargetColour(0, 255, 0);
		else if (newColour == 4)
		  setNewTargetColour(0, 255, 255);
		else if (newColour == 5)
		  setNewTargetColour(0, 0, 255);
		else if (newColour == 6)
		  setNewTargetColour(255, 0, 255);
		else if (newColour == 7)
		  setNewTargetColour(255, 255, 255);
	}

	if ((millis() - lastRxPacket) < 200)
	{
		digitalWrite(MSTR_LED_GRE, HIGH);
	}
	else
	{
		digitalWrite(MSTR_LED_GRE, LOW);
	}

	if (((millis() - colourUpdate) > 2) && (!testModeActive))
	{
		colourUpdate = millis();
		bool updateNeeded = false;
		if (targetRed > actualRed)
		{
			actualRed++;
			updateNeeded = true;
		}
		else if (targetRed < actualRed)
		{
			actualRed--;
			updateNeeded = true;
		}
		if (targetGre > actualGre)
		{
			actualGre++;
			updateNeeded = true;
		}
		else if (targetGre < actualGre)
		{
			actualGre--;
			updateNeeded = true;
		}
		if (targetBlu > actualBlu)
		{
			actualBlu++;
			updateNeeded = true;
		}
		else if (targetBlu < actualBlu)
		{
			actualBlu--;
			updateNeeded = true;
		}
		if (updateNeeded)
		{
			pixel.setPixelColor(0, pixel.Color(actualRed, actualGre, actualBlu));
			pixel.show();
		}
	}
}

int waitForDataOrTimeout(int numOfBytes)
{
	unsigned long startTime = millis();
	while (Serial.available() < numOfBytes)
	{
		if ((millis() - startTime) > (numOfBytes * 10))
			return RESULT_TIMEOUT;
	}
	return RESULT_DATA_RECEIVED;
}

void setNewTargetColour(byte red, byte gre, byte blu)
{
	targetRed = red;
	targetGre = gre;
	targetBlu = blu;
}

void storeButtonStatus(byte buttonAddress, byte c)
{
	/* We need to store the status of buttons upstream from us in the chain, */
	/* so we can rebroadcast their current state if an event happens to us.  */
	
	if (buttonAddress == 0)	//unassigned address
		return;

	for (int i = 0; i < (buttonAddress - 1); i++)
	{
		buttonState[i] = ((c >> (5 - i)) & 0x01);
	}
}
