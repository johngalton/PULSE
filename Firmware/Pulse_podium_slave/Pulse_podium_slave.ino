#include <Adafruit_NeoPixel.h>

#define MSTR_LED_GRE 3
#define MSTR_LED_YEL 4
#define SLVE_LED_GRE 2
#define SLVE_LED_YEL 5

#define BUT_SW 7
#define BUT_LED 6

#define DEBUG_BUT 10

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, BUT_LED, NEO_GRB + NEO_KHZ800);

byte targetRed = 0;
byte targetGre = 0;
byte targetBlu = 0;

byte actualRed = 0;
byte actualGre = 0;
byte actualBlu = 0;

unsigned long lastPress = 0;      // used for latch ing button presses for s specific time
unsigned long lastRxPacket = 0;   // used for the RJ45 jack status indicators
unsigned long colourUpdate = 0;   // used for the colour fading timings

byte buttonAddress = 0;   // 0 is undefined, 1-5 are podium numbers

int stickyTime = 200;

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
}

void loop()
{
	if (!digitalRead(BUT_SW))
		lastPress = millis();

   if ((millis() - lastPress) < stickyTime)
		digitalWrite(SLVE_LED_YEL, HIGH);
	else
		digitalWrite(SLVE_LED_YEL, LOW);


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
	}

  byte c = Serial.peek();

  if (c != -1)
  {
    byte newColour = 255;
  
    if (c != 0xFF)
      lastRxPacket = millis();
  
    if (c == 0xF0)        // first in chain, message from master
    {
      if (Serial.available() >= 6)
      {
        buttonAddress = 1;
        Serial.read();                // discard command byte
        newColour = Serial.read();    // use colour 1
        Serial.write(0xF1);
        Serial.write(Serial.read());  // send on colour 2
        Serial.write(Serial.read());  // send on colour 3
        Serial.write(Serial.read());  // send on colour 4
        Serial.write(Serial.read());  // send on colour 5
      }
    }
    else if (c == 0xF1)   // second in chain, message from button 1
    {
      if (Serial.available() >= 5)
      {
        buttonAddress = 2;
        Serial.read();                // discard command byte
        newColour = Serial.read();    // use colour 2
        Serial.write(0xF2);
        Serial.write(Serial.read());  // send on colour 3
        Serial.write(Serial.read());  // send on colour 4
        Serial.write(Serial.read());  // send on colour 5
       }
    }
    else if (c == 0xF2)   // third in chain, message from button 2
    {
      if (Serial.available() >= 4)
      {
        buttonAddress = 3;
        Serial.read();                // discard command byte
        newColour = Serial.read();    // use colour 3
        Serial.write(0xF3);
        Serial.write(Serial.read());  // send on colour 4
        Serial.write(Serial.read());  // send on colour 5
      }
    }
    else if (c == 0xF3)   // etc
    {
      if (Serial.available() >= 3)
      {
        buttonAddress = 4;
        Serial.read();                // discard command byte
        newColour = Serial.read();    // use colour 4
        Serial.write(0xF4);
        Serial.write(Serial.read());  // send on colour 5
      }
    }
    else if (c == 0xF4)
    {
      if (Serial.available() >= 2)
      {
        buttonAddress = 5;
        Serial.read();                // discard command byte
        newColour = Serial.read();    // use colour 5
        Serial.print("OK");           // send chain confimation
      }
    }
    else if (c == 0xFA)   // set hold time
    {
      if (Serial.available() >= 3)
      {
        Serial.read();  // discard command byte
        byte data1 = Serial.read();
        byte data2 = Serial.read();
        stickyTime = ((int)data1 << 8) | (int)data2;
        Serial.write(0xFA);
        Serial.write(data1);
        Serial.write(data2);
      }
    }
    else if ((c & 0xE0) == 0xC0) //request data command (top 3 bits are 110)
    {
      byte inData = Serial.read();
  
      if (((millis() - lastPress) < stickyTime) && (buttonAddress != 0))
        Serial.write(inData | (1 << (5 - buttonAddress)));
      else
        Serial.write(inData);
  
      lastPress = 0;    // reset sticky bit
    }
    else
    {
      if (c != 0xFF)
        Serial.read();  // discard first byte in buffer, as we didn't recognise it
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

	if ((millis() - colourUpdate) > 2)
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

void setNewTargetColour(byte red, byte gre, byte blu)
{
	targetRed = red;
	targetGre = gre;
	targetBlu = blu;
}
