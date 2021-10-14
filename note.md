- please remember to call matrix.show()
- set rotation to fit your LED strip
  - matrix.setRotation(2)
- external control
- arduino timer
- MQTT communication

## ble_test

* all led on -> 24mA
* all led off -> 15mA
* save power mode -> 20uA

## Timer

* When you're using `delay()` call, the `timer0` will be used. And this call will destroy other timer funcion somehow.

```c
unsigned long micros() {
  unsigned long m;
	uint8_t oldSREG = SREG, t;
	
	cli();
	m = timer0_overflow_count;
}
```
