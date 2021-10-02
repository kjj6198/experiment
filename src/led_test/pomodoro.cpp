#include "pomodoro.h"

Pomodoro::Pomodoro()
{
	this->state = pomodoro_state::idle;
	this->countdown = 10; // 60 * 30 as default
	this->working_time = 5;
	this->rest_time = 5;
};

void Pomodoro::disable_timer()
{
	cli();
	TCCR1B &= ~(1 << CS12);
	TCCR1B &= ~(1 << CS11);
	TCCR1B &= ~(1 << CS10);
	sei();
};

// pomodoro requires 16bit timer to do the timing
void Pomodoro::enable_timer()
{
	cli();
	TCCR1A = 0;
	TCCR1B = 0;
	OCR1A = 15625; // 1 secs
	TIMSK1 |= (1 << OCIE1A);
	TCCR1A |= (1 << WGM12);
	TCCR1B |= (1 << CS10) | (1 << CS12);
	sei();
};

void Pomodoro::begin()
{
	this->disable_timer();
};

// unit: seconds
int Pomodoro::start(int countdown, pomodoro_state state)
{
	if (countdown > 0)
	{
		this->state = state;
		this->countdown = countdown;
		this->enable_timer();
		return true;
	}

	return false;
};

void Pomodoro::decrement()
{
	this->countdown--;
};

void Pomodoro::set_per_second_callback(void (*fn)(void))
{
	this->persec_fn = fn;
};

void Pomodoro::pause()
{
	if (this->state == pomodoro_state::paused)
	{
		return;
	}
	this->state == pomodoro_state::paused;
	this->disable_timer();
}

void Pomodoro::resume()
{
	if (this->state == pomodoro_state::working)
	{
		return;
	}

	this->state == pomodoro_state::working;
	this->enable_timer();
}

void Pomodoro::reset()
{
	this->disable_timer();
	this->state = pomodoro_state::idle;
	this->countdown = this->working_time;
	TCNT1 = 0;
}

int Pomodoro::get_countdown()
{
	return this->countdown;
}

pomodoro_state Pomodoro::get_state()
{
	return this->state;
}


void Pomodoro::set_timesup_callback(void (*fn) (pomodoro_state state)) {
	this->timesup_fn = fn;
}

void Pomodoro::handle_timer_interrupt()
{
	TCNT1 = 0;
	if (this->countdown > 0)
	{
		if (this->persec_fn != NULL)
		{
			persec_fn();
		}
		this->countdown -= 1;
	}
	else if (this->countdown <= 0)
	{
		this->disable_timer();

		if (this->state == pomodoro_state::working)
		{
			this->state = pomodoro_state::rest;
			if (this->timesup_fn != NULL)
			{
				this->timesup_fn(pomodoro_state::rest);
			}
			this->countdown = this->rest_time;
			this->start(this->countdown, pomodoro_state::rest);
		}
		else if (this->state == pomodoro_state::rest)
		{
			this->state = pomodoro_state::paused;
			if (this->timesup_fn != NULL)
			{
				this->timesup_fn(pomodoro_state::idle);
			}
			this->reset();
		}
	}
}