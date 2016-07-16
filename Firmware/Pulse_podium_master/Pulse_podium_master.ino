#define MSTR_LED_GRE 3
#define MSTR_LED_YEL 4
#define SLVE_LED_GRE 2
#define SLVE_LED_YEL 5

#define BUT_SW 7
#define BUT_LED 6

#define NANO_RX 0
#define NANO_TX 1

bool prevNANO_RX;
bool prevNANO_TX;

unsigned long lastRXtoggle = 0;
unsigned long lastTXtoggle = 0;

void setup()
{
	pinMode(MSTR_LED_GRE, OUTPUT);
	pinMode(MSTR_LED_YEL, OUTPUT);
	pinMode(SLVE_LED_GRE, OUTPUT);
	pinMode(SLVE_LED_YEL, OUTPUT);

	pinMode(BUT_SW, INPUT);
	pinMode(BUT_LED, OUTPUT);

	pinMode(NANO_RX, INPUT);
	pinMode(NANO_TX, INPUT);

	prevNANO_RX = digitalRead(NANO_RX);
	prevNANO_TX = digitalRead(NANO_TX);
}

void loop()
{
	bool newNANO_RX = digitalRead(NANO_RX);
	bool newNANO_TX = digitalRead(NANO_TX);

	if (newNANO_RX != prevNANO_RX)
		lastRXtoggle = millis();

	if (newNANO_TX != prevNANO_TX)
		lastTXtoggle = millis();

	if ((millis() - 200) < lastRXtoggle)
		digitalWrite(SLVE_LED_GRE, HIGH);
	else
		digitalWrite(SLVE_LED_GRE, LOW);

	if ((millis() - 200) < lastTXtoggle)
		digitalWrite(SLVE_LED_YEL, HIGH);
	else
		digitalWrite(SLVE_LED_YEL, LOW);
}