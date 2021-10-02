#include "LedControl.h"
#include "pomodoro.h"

/*
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl lc = LedControl(4, 6 ,5 ,1);
Pomodoro pomodoro = Pomodoro();

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

void callback() {
  Serial.println(pomodoro.get_countdown());
}

void complete(pomodoro_state state) {
  if (state == pomodoro_state::rest) {
    Serial.println("now let's get a rest");
  } else if (state == pomodoro_state::idle) {
    Serial.println("You just completed a pomodoro, nice!");
  }
}

void setup() {
  Serial.begin(115200);
  pomodoro.set_per_second_callback(callback);
  pomodoro.set_timesup_callback(complete);

  pomodoro.start(15, pomodoro_state::working);
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
  pomodoro.handle_timer_interrupt();
}