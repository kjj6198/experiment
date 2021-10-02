#include "LedControl.h"

/*
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl lc = LedControl(4, 6 ,5 ,1);

volatile int count = 0;
volatile int mat[] = {
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1
};

volatile int currI = 0;
volatile int currJ = 0;
volatile int state = 0;

void startTimer() {
	cli();
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
	OCR1A = 100;
	TIMSK1 |= (1 << OCIE1A);
	TCCR1A |= (1 << WGM12);
	TCCR1B |= (1 << CS10) | (1 << CS12);
	sei(); 
}

void setup() {
  startTimer();
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 0.2);
  /* and clear the display */
  lc.clearDisplay(0);
  render();
  
  Serial.begin(115200);
}

void render() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      lc.setLed(0, i,  7 - j, mat[i * 8 + j]);
    }
  }
}

void blinkOne() {
  if (state == 0) {
    state = 1;
    lc.setLed(0, currJ, 7 - currI, 1);
  } else {
    state = 0;
    lc.setLed(0, currJ, 7 - currI, 0);
  }
}

void loop() { 
	blinkOne();
  delay(500);
}


ISR(TIMER1_COMPA_vect) {
  TCNT1 = 0;
  if (count == 28) {
    count = 0;
    mat[currJ * 8 + currI] = 0;
    if (currJ == 7) {
      currI += 1;
    }
    currJ = (currJ + 1) % 8;
    render();
  }
  
  count++;
}