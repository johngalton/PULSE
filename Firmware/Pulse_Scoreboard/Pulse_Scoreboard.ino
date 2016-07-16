#define SHIFT_CLOCK 3
#define SHIFT_DATA 5
#define nSHIFT_OE 9

long scoreCount = -1;

char ledDisplay[9];  //user modifiable
byte outData[8];  //don't touch this, auto populated

byte brightness = 200;  //0 to 255

char serBuf[64];
int bufPos = -1;

long countSteps = 30;  //at 10mS per step

bool useStartChar = false;

bool showLeadingZeros = true;

void setup()
{   
	pinMode(SHIFT_DATA, OUTPUT);
	pinMode(nSHIFT_OE, OUTPUT);
	pinMode(SHIFT_CLOCK, OUTPUT);

  clearDisplay();
  TCCR1B = TCCR1B & 0b11111000 | 0x01;  //set OE PWM frequency to 31.25kHz
  
  delay(50);

  pulse();
    
  Serial.begin(9600);
}

void loop()
{
  if (Serial.available())
  {
    if ((bufPos != -1) || (useStartChar == false))   //we have already started
    {
      if (bufPos == -1)
        bufPos = 0;
      char inChar = Serial.read();
      if ((inChar == 13) || (inChar == 10)) //EOL
      {
        serBuf[bufPos] = 0;      //null terminate

        //Handle received string

        char* command;
        char* data;        
        command = strtok(serBuf, "=");
        data = strtok(NULL, "=");

        if ((command != NULL) && (data != NULL))
        {
          if (strstr(command, "text") != NULL)
          {
            Serial.println("OK");
            setDisplayText(data);
            scoreCount = -1;
          }
          else if (strstr(command, "numb") != NULL)
          {
            Serial.println("OK");
            scoreCount = atol(data);
            setDisplayNum(scoreCount);
          }
          else if (strstr(command, "score") != NULL)
          {
            long newScore = atol(data);
            
            Serial.println("OK");
            if (scoreCount == -1)
            {
              setDisplayNum(newScore);
              scoreCount = newScore;
            }
            else
            {
              long stepSize;
              if (scoreCount < newScore)
              {
                stepSize = ((newScore - scoreCount) / countSteps);
                if (stepSize == 0)
                  stepSize = 1;
                while (scoreCount < (newScore - stepSize))
                {
                  setDisplayNum(scoreCount);
                  delay(10);
                  scoreCount += stepSize;
                }
                scoreCount = newScore;
                setDisplayNum(scoreCount);
              }
              else
              {
                stepSize = ((scoreCount - newScore) / countSteps);
                if (stepSize == 0)
                  stepSize = 1;
                while (scoreCount > (newScore + stepSize))
                {
                  setDisplayNum(scoreCount);
                  delay(10);
                  scoreCount -= stepSize;
                }
                scoreCount = newScore;
                setDisplayNum(scoreCount);
              }
            }
          }
          else if (strstr(command, "delta") != NULL)
          {
            if (scoreCount != -1)
            {
              Serial.println("OK");
              long target = scoreCount + atol(data);
              long stepSize = ((target - scoreCount) / countSteps);
              if (stepSize == 0)
                  stepSize = 1;
              while (scoreCount < (target - stepSize))
              {
                setDisplayNum(scoreCount);
                delay(10);
                scoreCount += stepSize;
              }
              scoreCount = target;
              setDisplayNum(scoreCount);
            }
            else
            {
              Serial.println("Score not currently displayed");
            }
          }
          else if (strstr(command, "countTime") != NULL)
          {
            countSteps = (atoi(data) / 10);
            Serial.println("OK");
          }
          else if (strstr(command, "showLeadingZeros") != NULL)
          {
            if ((data[0] == '0') || (data[0] == 'n'))
            {
              showLeadingZeros = false;
              if (scoreCount != -1)
                setDisplayNum(scoreCount);
              Serial.println("OK");
            }
            else if ((data[0] == '1') || (data[0] == 'y'))
            {
              showLeadingZeros = true;
              if (scoreCount != -1)
                setDisplayNum(scoreCount);
              Serial.println("OK");
            }
            else
            {
              Serial.println("Invalid parameter");
            }
          }
          else if (strstr(command, "bright") != NULL)
          {
            int newBrightness = atoi(data);
            if ((newBrightness >= 0) && (newBrightness <= 255))
            {
              brightness = newBrightness;
              updateBrightness();
              Serial.println("OK");
            }
            else
            {
              Serial.println("Out of range");
            }
          }
          else
          {
            Serial.println("Invalid command");
          }
        }
        else if (strstr(command, "clear") != NULL)
        {
          Serial.println("OK");
          clearDisplay();
          scoreCount = -1;
        }
        else if (strstr(command, "pulse") != NULL)
        {
          Serial.println("OK");
          pulse();
          scoreCount = -1;
        }
        else
        {
          if (strlen(serBuf) > 0)
            Serial.println("Invalid command format");
        }
        bufPos = -1;
      }
      else
      {
        serBuf[bufPos] = inChar;
        bufPos++;
      }
    }
    else
    {
      if (Serial.read() == '!') //start char
        bufPos = 0;
    }
  }

}




void setDisplayNum(unsigned long inValue)
{
	byte digits[8];
	digits[7] = (inValue % 10);
	digits[6] = ((inValue / 10) % 10);
	digits[5] = ((inValue / 100) % 10);
	digits[4] = ((inValue / 1000) % 10);
	digits[3] = ((inValue / 10000) % 10);
	digits[2] = ((inValue / 100000) % 10);
	digits[1] = ((inValue / 1000000) % 10);
	digits[0] = ((inValue / 10000000) % 10);

  bool allZerosSoFar = true;

	for (int i = 0; i < 8; i++)
  {
    if ((digits[i] == 0) && (allZerosSoFar == true) && (showLeadingZeros == false))
    {
      digits[i] = ' ';
    }
    else
    {
      digits[i] = digits[i] + 48;
      allZerosSoFar = false;
    }
  }

	setDisplayText((char*)digits);
}

void setDisplayText(char * displayData)
{
  byte numbers[10] = { 0x7e, 0x0c, 0xb6, 0x9e, 0xcc, 0xda, 0xfa, 0x0e, 0xfe, 0xde };  //starts at ascii 48

  //starts at ascii 65  A      B     C     D     E     F     G     H     I     J     K     L     M     N     O     P     Q     R     S     T     U     V     W     X     Y     Z     [     \     ]     ^     _     '     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o     p     q     r     s     t     u     v     w     x     y     z
  byte letters[57] = { 0xee, 0xfe, 0x72, 0x7e, 0xf2, 0xe2, 0x7a, 0xec, 0x0C, 0x3c, 0x80, 0x70, 0x80, 0x6e, 0x7e, 0xe6, 0xce, 0x62, 0xda, 0xf0, 0x7c, 0x7c, 0x80, 0x00, 0xdc, 0x80, 0x72, 0xa4, 0x1e, 0x02, 0x10, 0x01, 0xbe, 0xf8, 0xb0, 0xbc, 0xf6, 0xe2, 0xde, 0xe8, 0x60, 0x1C, 0x80, 0x70, 0x80, 0xa8, 0xb8, 0xe6, 0xce, 0xa0, 0xda, 0xf0, 0x38, 0x38, 0x80, 0x80, 0xdc };

  for (int i = 0; i < 8; i++)
    outData[i] = 0;

  for (int i = 0; i < 8; i++)
  {
    if (displayData[i] == 0)   // NULL TERMINATOR
      break;
    if (displayData[i] == 32) // SPACE
      outData[7 - i] = 0;
    if ((displayData[i] >= 48) && (displayData[i] <= 57))
      outData[7 - i] = (numbers[displayData[i] - 48]);
    else if ((displayData[i] >= 65) && (displayData[i] <= 121))
      outData[7 - i] = (letters[displayData[i] - 65]);
  }

  sendOutData();
}

void sendOutData()
{	
	analogWrite(nSHIFT_OE, 255);
  
	for (int character = 0; character < 8; character++)
	{
		/*
    ON A NANO:
		SHIFT_DATA is PIN5, which maps to PD5
		SHIFT_CLOCK is PIN3, which maps to PD3

    ON A MICRO:
    SHIFT_DATA is PIN5, which maps to PC6
    SHIFT_CLOCK is PIN3, which maps to PD0
		*/

		for (int bit = 7; bit >= 0; bit--)
		{
			if ((outData[character] >> bit) & 0x01)
				PORTC = PORTC | B01000000;
			else
				PORTC = PORTC & B10111111;
			__asm__("nop\n\t");
			PORTD = PORTD | B00000001;
			__asm__("nop\n\t");
			PORTD = PORTD & B11111110;
		}
	}
  
	updateBrightness();
}

void clearDisplay()
{
  analogWrite(nSHIFT_OE, 255);
  PORTC = PORTC & B10111111;
  for (int i = 0; i < 65; i++)
  {
    __asm__("nop\n\t");
    PORTD = PORTD | B00000001;
    __asm__("nop\n\t");
    PORTD = PORTD & B11111110;
  }
  
  for (int i = 0; i < 8; i++)
    ledDisplay[i] = ' ';
    
  updateBrightness();
}

void updateBrightness()
{
  analogWrite(nSHIFT_OE, 255 - brightness);
}

void pulse()
{
  byte oldBrightness = brightness;
  brightness = 1;
  clearDisplay();
  
  ledDisplay[1] = 'P';
  ledDisplay[2] = 'U';
  ledDisplay[3] = 'L';
  ledDisplay[4] = 'S';
  ledDisplay[5] = 'E';
  setDisplayText(ledDisplay);

  while (brightness < 255)
  {
    updateBrightness();
    brightness++;
    delay(3);
  }

  delay(50);

  while (brightness > 0)
  {
    updateBrightness();
    if (brightness > 150)
      brightness--;
    brightness--;
    delay(3);
  }

  brightness = oldBrightness;
  clearDisplay();
}

