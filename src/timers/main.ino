

void setup()
{
	Serial.begin(9600);
	cli();
	TCNT1 = 0;
	TCCR1A = 0;
	TCCR1B = 0;
	OCR1A = 1000; // 1 secs
	TIMSK1 |= (1 << OCIE1A);
	TCCR1A |= (1 << WGM12);
	TCCR1B |= (1 << CS10) | (1 << CS12);
	sei();

	// cli();
	TCCR2A = 0;
	TCCR2B = 0;
	TCNT2 = 0;
	OCR2A = 255;
	TIMSK2 |= 1 << TOIE2;
	TCCR2A |= 1 << WGM21;
	TCCR2B |= (1 << CS22) | (1 << CS20);
	// sei();
}

void loop()
{
	Serial.println("FUFUFUFU");
	delay(500);
}

ISR(TIMER1_COMPA_vect)
{
	Serial.println("timer1");
}

int count = 0;
ISR(TIMER2_OVF_vect) {
	if (count == 1000) {
		Serial.println("timer2");
		count = 0;
	}
	count++;
}

