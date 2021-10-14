#include "pomodoro.h"

Pomodoro::Pomodoro()
{
	this->state = pomodoro_state::idle;
	this->countdown = 0; // 60 * 30 as default
	// TODO: parameterize
	this->working_time = 1800;
	this->rest_time = 300;
	this->overflow_count = 0;
};

void Pomodoro::disable_timer()
{
	cli();	
	TCCR1B &= ~(1 << CS12);
	TCCR1B &= ~(1 << CS11);
	TCCR1B &= ~(1 << CS10);
	TCCR2B &= ~(1 << CS22);
	TCCR2B &= ~(1 << CS21);
	TCCR2B &= ~(1 << CS20);
	sei();
};

// pomodoro requires 16bit timer to do the timing
void Pomodoro::enable_timer()
{
	cli();
	TCCR1A = 0;
	TCCR1B = 0;
	OCR1A = 7812; // 1 secs
	TIMSK1 |= (1 << OCIE1A);
	TCCR1A |= (1 << WGM12);
	TCCR1B |= (1 << CS10) | (1 << CS12);
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
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
	if (this->is_running())
	{
		this->disable_timer();
	}
}

void Pomodoro::resume()
{
	if (this->is_running())
	{
		return;
	}

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

void Pomodoro::set_timesup_callback(void (*fn)(pomodoro_state state))
{
	this->timesup_fn = fn;
}

void Pomodoro::set_half_second_callback(void (*fn)(void))
{
	this->halfsec_fn = fn;
}

void Pomodoro::handle_timer_interrupt()
{
	TCNT1 = 0;
	if (this->overflow_count == 1)
	{
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
				if (this->timesup_fn != NULL)
				{
					this->timesup_fn(pomodoro_state::idle);
				}
				this->reset();
			}
		}
	}

	if (this->halfsec_fn != NULL)
	{
		halfsec_fn();
	}

	this->overflow_count = (this->overflow_count + 1) % 2;
}

bool Pomodoro::is_running()
{
	if (
			TCCR1B & (1 << CS12) == (1 << CS12) &&
			TCCR1B & (1 << CS11) == (1 << CS11) &&
			TCCR1B & (1 << CS10) == (1 << CS10))
	{
		return true;
	}
	return false;
}