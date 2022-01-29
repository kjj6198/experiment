#include "LedControl.h"
#include "pomodoro.h"
#include "SoftwareSerial.h"
#include "packet.h"

#define POMODORO_TIME 7 * 60
#define BREAK_TIME 3 * 60
#define COUNT 30
#define PIN_TX 5
#define PIN_RX 4
#define WORKING_LED_PIN 7
#define BREAK_LED_PIN 8
#define BUZZER_PIN 13

#define ledInterval(p) (p / 64)

/*
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl lc = LedControl(12, 10, 11, 1);

Pomodoro pomodoro = Pomodoro();
SoftwareSerial ble_serial = SoftwareSerial(PIN_RX, PIN_TX);

volatile int count = 0;
volatile int mat[] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1};

volatile int currI = 0;
volatile int currJ = 0;
volatile int state = 0;
volatile unsigned long curr = 0;
volatile int timer_overflow_count = 0;
volatile int target_overflow_count = 0;
volatile uint8_t remain_tick = 0;
volatile uint8_t tick = 0;

void test()
{
  blinkOne();
}

void callback()
{
  int state = pomodoro.get_state();
  if (state == pomodoro_state::working)
  {
    digitalWrite(WORKING_LED_PIN, HIGH);
    digitalWrite(BREAK_LED_PIN, LOW);
  }
  else if (state == pomodoro_state::rest)
  {
    digitalWrite(WORKING_LED_PIN, LOW);
    digitalWrite(BREAK_LED_PIN, HIGH);
  }
  else
  {
    digitalWrite(WORKING_LED_PIN, LOW);
    digitalWrite(BREAK_LED_PIN, LOW);
  }
  Serial.println("second callback");

  int remaining = pomodoro.get_countdown();
  uint8_t l = remaining & 0xff;
  uint8_t h = (remaining >> 8) & 0xff;

  uint8_t res[4] = {
      0x07, h, l, state};
  ble_serial.write(res, 4);
}

void buzz() {
  tone(BUZZER_PIN, 1200);
  delay(1000);
  tone(BUZZER_PIN, 1200);
  delay(1000);
  noTone(BUZZER_PIN);
}

void complete(pomodoro_state state)
{
  buzz();
  if (state == pomodoro_state::rest)
  {
    Serial.println("now let's get a rest");

    uint8_t data[4] = {
        0x07, 0x00, 0x00, state};

    ble_serial.write(data, 4);
    currI = 7;
    currJ = 7;

    setupLedTimerInterval(BREAK_TIME / 64);
  }
  else if (state == pomodoro_state::idle)
  {
    uint8_t data[4] = {
        0x07, 0x00, 0x00, 0xff};

    ble_serial.write(data, 4);
    Serial.println("You just completed a pomodoro, nice!");
  }

  if (state == pomodoro_state::working)
  {
    digitalWrite(WORKING_LED_PIN, HIGH);
    digitalWrite(BREAK_LED_PIN, LOW);
  }
  else if (state == pomodoro_state::rest)
  {
    digitalWrite(WORKING_LED_PIN, LOW);
    digitalWrite(BREAK_LED_PIN, HIGH);
  }
  else
  {
    digitalWrite(WORKING_LED_PIN, LOW);
    digitalWrite(BREAK_LED_PIN, LOW);
  }
}

// this function will iterate matrix and set the leds
// a potentional optimization would be to only update the leds that have changed
void render()
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      lc.setLed(0, i, 7 - j, mat[i * 8 + j]);
    }
  }
}

void setup()
{
  pinMode(WORKING_LED_PIN, OUTPUT);
  pinMode(BREAK_LED_PIN, OUTPUT);

  setupLedTimerInterval(POMODORO_TIME / 64);
  pomodoro.set_per_second_callback(callback);
  pomodoro.set_timesup_callback(complete);
  pomodoro.set_half_second_callback(test);

  // needed to wake up the MAX72XX
  lc.shutdown(0, false);
  lc.setIntensity(0, 1);
  lc.clearDisplay(0);
  render();

  Serial.begin(9600);
  ble_serial.begin(9600);

  pinMode(2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), handle_button_clicked, RISING);
}

void send_packet(int running)
{
  int remaining = pomodoro.get_countdown();
  uint8_t state = pomodoro.get_state();
  uint8_t l = remaining & 0xff;
  uint8_t h = (remaining >> 8) & 0xff;
  uint8_t is_running = running & 0x01;

  uint8_t res[4] = {
      0x07, h, l, is_running };

  ble_serial.write(res, 4);
}

void handle_button_clicked()
{

#ifdef DEBUG
  Serial.println("trigger");
  char buf2[20];
  sprintf(buf2, "state: %d, countdown: %d", state, pomodoro.get_countdown());
#endif
  int state = pomodoro.get_state();
  int countdown = pomodoro.get_countdown();

  if (state == pomodoro_state::idle)
  {
    currI = 0;
    currJ = 0;
    pomodoro.start(POMODORO_TIME, pomodoro_state::working);
    send_packet(1);
  }
  else if (state == pomodoro_state::working || state == pomodoro_state::rest)
  {
    if (pomodoro.is_running())
    {
      pomodoro.pause();
      send_packet(0);
    }
    else
    {
      pomodoro.resume();
      send_packet(1);
    }
  }
}

void blinkOne()
{
  if (state == 0)
  {
    state = 1;
    lc.setLed(0, currJ, 7 - currI, 1);
  }
  else
  {
    state = 0;
    lc.setLed(0, currJ, 7 - currI, 0);
  }
}

char c = ' ';

void handle_command()
{
  uint8_t d[4];
  for (int i = 0; i < 4; i++)
  {
    d[i] = ble_serial.read();
  }

  if (d[0] < 0x0a)
  {
    packet_t p;
    parse_packet(d, &p);
    Serial.println(p.command);
    uint8_t ok[4] = {
        0x09, 0xff, 0xff, 0xff};

    char buffer[20];
    

    if (p.command == "start")
    {
      currI = 0;
      currJ = 0;
      pomodoro.start(POMODORO_TIME, pomodoro_state::working);
    }
    else if (p.command == "pause")
    {
      pomodoro.pause();
    }
    else if (p.command == "resume")
    {
      pomodoro.resume();
    }
    else if (p.command == "reset")
    {
      pomodoro.reset();
    }
    else if (p.command == "ask")
    {
      int remaining = pomodoro.get_countdown();
      uint8_t state = pomodoro.get_state();
      uint8_t l = remaining & 0xff;
      uint8_t h = (remaining >> 8) & 0xff;

      uint8_t res[4] = {
          0x07, h, l, state};
      ble_serial.write(res, 4);
    }
  }
}

void loop()
{
  if (ble_serial.available())
  {
    handle_command();
    ble_serial.flush();
  }
  delay(100);
}

// FIXME: if timer2 is enable, then timer1 somehow doesn't work (stop counting)
// note: it turns out it's just because the interrupt is excuted too fast and
// the serial just can't catch the message correctly.
void setupLedTimerInterval(float interval)
{
  long cyclesOfOneSecond = (F_CPU / 1024);
  long totalTicks = interval * cyclesOfOneSecond;
  uint16_t overflowCount = totalTicks / 256;
  remain_tick = totalTicks % 256;

  TCCR2A = 0;
  TCCR2B = 0;

  TCNT2 = 0;
  OCR2A = 255;
  TIMSK2 |= 1 << TOIE2;
  TCCR2A |= 1 << WGM21;
  TIFR2 = 1 << OCF2B;
  target_overflow_count = overflowCount;
  Serial.print("count:");
  Serial.println(overflowCount);
}

inline void updateLed()
{
  if (pomodoro.get_state() == pomodoro_state::working)
  {
    mat[currJ * 8 + currI] = 0;
    if (currJ == 7)
    {
      currI += 1;
    }
    currJ = (currJ + 1) % 8;
    render();
  }
  else if (pomodoro.get_state() == pomodoro_state::rest)
  {
    mat[currJ * 8 + currI] = 1;
    if (currJ == 0)
    {
      currI -= 1;
    }

    if (currJ - 1 < 0) {
      currJ = 7;
    } else {
      currJ = (currJ - 1) % 8;
    }
    
    
    render();
  }
}

ISR(TIMER2_OVF_vect)
{
  if (timer_overflow_count == target_overflow_count)
  {
    Serial.println("timer overflow");
    updateLed();
  }
  timer_overflow_count = (timer_overflow_count + 1) % (target_overflow_count + 1);
}

ISR(TIMER1_COMPA_vect)
{
  pomodoro.handle_timer_interrupt();
}