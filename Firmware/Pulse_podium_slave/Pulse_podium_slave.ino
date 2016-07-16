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

unsigned long lastPress = 0;
unsigned long lastValidPacket = 0;

unsigned long colourUpdate = 0;

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
	{
		lastPress = millis();
		digitalWrite(SLVE_LED_YEL, HIGH);
	}
	else
	{
		digitalWrite(SLVE_LED_YEL, LOW);
	}


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


	if (Serial.available() >= 6)	// START LED1 LED2 LED3 LED4 LED 5
	{
		if (Serial.read() == 127)	// START BYTE
		{
			lastValidPacket = millis();
			byte data1 = Serial.read();
			byte data2 = Serial.read();
			byte data3 = Serial.read();
			byte data4 = Serial.read();
			byte data5 = Serial.read();

			if (data1 == 0)
				setNewTargetColour(0, 0, 0);
			else if (data1 == 1)
				setNewTargetColour(255, 0, 0);
			else if (data1 == 2)
				setNewTargetColour(255, 255, 0);
			else if (data1 == 3)
				setNewTargetColour(0, 255, 0);
			else if (data1 == 4)
				setNewTargetColour(0, 255, 255);
			else if (data1 == 5)
				setNewTargetColour(0, 0, 255);
			else if (data1 == 6)
				setNewTargetColour(255, 0, 255);
			else if (data1 == 7)
				setNewTargetColour(255, 255, 255);

			Serial.write(127);
			Serial.write(data2);
			Serial.write(data3);
			Serial.write(data4);
			Serial.write(data5);
			if ((millis() - lastPress) < 200)
				Serial.write(1);
			else
				Serial.write(0);
		}
	}

	if ((millis() - lastValidPacket) < 200)
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