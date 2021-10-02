#include <SoftwareSerial.h>

#define PIN_TX 3
#define PIN_RX 2

SoftwareSerial ble_serial(PIN_RX, PIN_TX);

void setup()
{
	Serial.begin(9600);
	ble_serial.begin(9600);
}

char c = ' ';
int count = 10;

void loop()
{
	if (ble_serial.available())
	{
		c = ble_serial.read();
		Serial.write(c);
	}

	if (Serial.available())
	{
		char ch = Serial.read();
		ble_serial.write(ch);
	}

	delay(1000);
	count -= 1;
	ble_serial.write(count);
}