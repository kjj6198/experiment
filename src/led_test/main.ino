#include "LedControl.h"
#include "pomodoro.h"
#include "SoftwareSerial.h"
#include "packet.h"

#define COUNT 120
#define PIN_TX 5
#define PIN_RX 4
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

void callback()
{
  if (count >= 2) {
    count = 0;
    mat[currJ * 8 + currI] = 0;
    if (currJ == 7)
    {
      currI += 1;
    }
    currJ = (currJ + 1) % 8;
    render();
  }

  count++;
}

void complete(pomodoro_state state)
{
  
  if (state == pomodoro_state::rest)
  {
    Serial.println("now let's get a rest");

    uint8_t data[4] = {
        0x07, 0x00, 0x00, state};

    ble_serial.write(data, 4);
  }
  else if (state == pomodoro_state::idle)
  {
    uint8_t data[4] = {
        0x07, 0x00, 0x00, 0xff};

    ble_serial.write(data, 4);
    Serial.println("You just completed a pomodoro, nice!");
  }

  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      mat[i * 8 + j] = 1;
    }
  }
  currI = 0;
  currJ = 0;
}

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
  lc.shutdown(0, false);
  lc.setIntensity(0, 1);
  lc.clearDisplay(0);
  render();
  pinMode(2, INPUT_PULLUP);
  
  Serial.begin(9600);
  ble_serial.begin(9600);
  pomodoro.set_per_second_callback(callback);
  pomodoro.set_timesup_callback(complete);

  attachInterrupt(digitalPinToInterrupt(2), handle_button_clicked, RISING);
}

void handle_button_clicked()
{

#ifdef DEBUG
  Serial.println("trigger");
#endif
  int state = pomodoro.get_state();
  int countdown = pomodoro.get_countdown();

  char buf2[20];
  sprintf(buf2, "state: %d, countdown: %d", state, pomodoro.get_countdown());
  Serial.println(buf2);

  Serial.println(pomodoro.is_running());

  if (state == pomodoro_state::idle)
  {
    pomodoro.start(COUNT, pomodoro_state::working);
  }
  else if (state == pomodoro_state::working || state == pomodoro_state::rest)
  {
    if (pomodoro.is_running())
    {
      pomodoro.pause();
    }
    else if (countdown != 0)
    {
      pomodoro.resume();
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

inline void handle_command(uint8_t *d)
{
  uint8_t data[4];
  // ble_serial.readBytes(data, 4);

  if (data[0] < 0x0a)
  {
    packet_t p;
    parse_packet(d, &p);
    Serial.println(p.command);

    uint8_t ok[4] = {
        0x09, 0xff, 0xff, 0xff};

    if (p.command == "start")
    {
      pomodoro.start(COUNT, pomodoro_state::working);
      ble_serial.write(ok, 4);
    }
    else if (p.command == "pause")
    {
      pomodoro.pause();
      ble_serial.write(ok, 4);
    }
    else if (p.command == "resume")
    {
      pomodoro.resume();
      ble_serial.write(ok, 4);
    }
    else if (p.command == "reset")
    {
      pomodoro.reset();
      ble_serial.write(ok, 4);
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
  // if (ble_serial.available())
  // {
  //   uint8_t data[4];
  //   ble_serial.readBytes(data, 4);
  //   handle_command(data);
  // }

  uint8_t data[4] = {
    0x01, 0x00, 0x00, 0x00
  };

  // handle_command(data);
  // pomodoro.start(COUNT, pomodoro_state::working);
  // blinkOne();
  delay(30000);
}

ISR(TIMER1_COMPA_vect)
{
  pomodoro.handle_timer_interrupt();
}