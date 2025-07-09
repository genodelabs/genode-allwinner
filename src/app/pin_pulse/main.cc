/*
 * \brief  Driving a pin crude pulse-code modulation
 * \author Norman Feske
 * \date   2021-04-22
 */

/* Genode includes */
#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <timer_session/connection.h>
#include <pin_control_session/connection.h>

namespace Pin_pulse {

	using namespace Genode;

	struct Main;
}


struct Pin_pulse::Main
{
	Env &_env;

	Pin_control::Connection _pin { _env };

	Timer::Connection _timer { _env };

	Signal_handler<Main> _timer_handler {
		_env.ep(), *this, &Main::_handle_timer };

	void _handle_timer();


	/*
	 * Configuration
	 */

	unsigned _pcm_period_ms = 0;
	unsigned _pulse_ms  = 0;

	Attached_rom_dataspace _config { _env, "config" };

	Signal_handler<Main> _config_handler {
		_env.ep(), *this, &Main::_handle_config };

	void _handle_config()
	{
		_config.update();

		_pcm_period_ms = _config.node().attribute_value("pcm_period_ms", 15U);
		_pulse_ms      = _config.node().attribute_value("pulse_ms",     500U);

		if (_pcm_period_ms == 0 || _pulse_ms == 0)
			warning("invalid configuration");

		_handle_timer();
	}


	/*
	 * Signal-level computation
	 */

	enum class Direction { UP, DOWN } _direction { Direction::UP };

	bool _curr_signal = false;

	/* ratio between low and high signal level, value between 0 and 1.0 */
	double _low_high_ratio = 0;

	unsigned _periods_per_pulse() const { return _pulse_ms / _pcm_period_ms; }

	/* gradual change of '_low_high_ratio' per period */
	double _ratio_step() const { return 1.0 / _periods_per_pulse(); }


	Main(Env &env) : _env(env)
	{
		_config.sigh(_config_handler);
		_handle_config();

		_timer.sigh(_timer_handler);
	}
};


void Pin_pulse::Main::_handle_timer()
{
	if (_pcm_period_ms == 0 || _pulse_ms == 0)
		return;

	/* rising edge */
	if (_curr_signal == 0) {

		/* adjust '_low_high_ratio' once per period, at rising edge */
		if (_direction == Direction::UP) {
			_low_high_ratio = _low_high_ratio + _ratio_step();
			if (_low_high_ratio > 1.0) {
				_low_high_ratio = 1.0;
				_direction = Direction::DOWN;
			}
		} else {
			_low_high_ratio = _low_high_ratio - _ratio_step();
			if (_low_high_ratio < 0) {
				_low_high_ratio = 0;
				_direction = Direction::UP;
			}
		}

		/* schedule falling edge */
		_timer.trigger_once(uint64_t(1000.0*_pcm_period_ms*_low_high_ratio));
	}

	/* falling edge */
	else {

		/* schedule falling edge */
		_timer.trigger_once(uint64_t(1000.0*_pcm_period_ms*(1.0 - _low_high_ratio)));
	}

	_curr_signal = !_curr_signal;

	_pin.state(_curr_signal);
}


void Component::construct(Genode::Env &env)
{
	static Pin_pulse::Main main(env);
}

