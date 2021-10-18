#include "Arduino.h"

#ifndef pomodoro_h
	#define pomodoro_h
#endif

enum pomodoro_state {
	idle = 0,
	working = 1,
	rest = 2,
	paused = 3
};

class Pomodoro {
	private :
		volatile int countdown;
		int working_time;
		int overflow_count;
		int rest_time;
		volatile pomodoro_state state;

		void decrement();
		void disable_timer();
		void enable_timer();
		void (*timesup_fn) (pomodoro_state state);
		void (*halfsec_fn) (void);
		void (*persec_fn) (void);

	public :
	  Pomodoro();

		void begin();
		int start(int countdown, pomodoro_state state);
		void pause();
		void reset();
		void resume();

		int get_countdown();
		int get_working_time();
		int get_rest_time();
		bool is_running();

		// this function will be passed into interrupt handler (ISR)
		// so be sure to not doing too much thing in callback function
		void set_timesup_callback(void (*fn) (pomodoro_state state));
		void set_per_second_callback(void (*fn) (void));
		void set_half_second_callback(void (*fn) (void));

		void handle_timer_interrupt();
		

		pomodoro_state get_state();
};