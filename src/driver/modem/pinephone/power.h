/*
 * \brief  Low-level modem power control
 * \author Norman Feske
 * \author Sebastian Sumpf
 * \date   2022-06-15
 *
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _POWER_H_
#define _POWER_H_

/* Genode includes */
#include <pin_control_session/connection.h>
#include <pin_state_session/connection.h>

/* local includes */
#include <types.h>

namespace Modem { struct Power; }


struct Modem::Power
{
	enum class Requested { DONT_CARE, OFF, ON };

	Requested _requested = Requested::DONT_CARE;

	enum class State { UNKNOWN, OFF, STARTING_UP, ON, SHUTTING_DOWN };

	State _state = State::UNKNOWN;

	Env &_env;

	Delayer &_delayer;

	Pin_state::Connection   _pin_status     { _env, "status"     };

	Pin_control::Connection _pin_battery    { _env, "battery"    },
	                        _pin_dtr        { _env, "dtr"        },
	                        _pin_enable     { _env, "enable"     },
	                        _pin_host_ready { _env, "host-ready" },
	                        _pin_pwrkey     { _env, "pwrkey"     },
	                        _pin_reset      { _env, "reset"      };

	void _press_pwrkey()
	{
		/* issue power key pulse >= 500ms */
		_pin_pwrkey.state(true);
		_delayer.msleep(600);
		_pin_pwrkey.state(false);
	}

	void _drive_power_up()
	{
		switch (_state) {

		case State::OFF:

			_pin_enable.state(false);   /* enable RF */

			log("Powering up modem ...");
			_press_pwrkey();
			_state = State::STARTING_UP;
			break;

		case State::STARTING_UP:
			if (_pin_status.state() == 0)
				_state = State::ON;
			break;

		case State::UNKNOWN:
		case State::ON:
		case State::SHUTTING_DOWN:
			break;
		}
	}

	void _drive_power_down()
	{
		switch (_state) {

		case State::UNKNOWN:
		case State::OFF:
			break;

		case State::STARTING_UP:
		case State::ON:

			_pin_enable.state(true);

			log("Powering down modem via power-key signal ...");
			_press_pwrkey();

			_state = State::SHUTTING_DOWN;
			break;

		case State::SHUTTING_DOWN:
			if (_pin_status.state() == 1)
				_state = State::OFF;
			break;
		}
	}

	Power(Env &env, Delayer &delayer) : _env(env), _delayer(delayer)
	{
		/*
		 * Note that by enabling '_pin_battery', the '_pin_status' changes
		 * from 0 (on) to 1 (off). This is not desired in cases where the
		 * modem should keep its state (e.g., PIN) across reboots.
		 *
		 * Open question: How to reliably establish the command channel to the
		 * modem when it is already powered?
		 */
		_pin_battery.state(true);

		_delayer.msleep(30);

		_pin_reset.state(false);
		_pin_host_ready.state(false);
		_pin_dtr.state(false);      /* no suspend */

		_delayer.msleep(30);
	}

	struct Condition
	{
		bool safe_to_power_off;
	};

	void power_enabled(bool enabled)
	{
		_requested = enabled ? Requested::ON : Requested::OFF;

		if (_state == State::UNKNOWN)
			_state = _pin_status.state() ? State::OFF : State::ON;

		for (;;) {
			State const orig_state = _state;

			switch (_requested) {
			case Requested::DONT_CARE:                break;
			case Requested::ON:  _drive_power_up();   break;
			case Requested::OFF: _drive_power_down(); break;
			}

			if (orig_state == _state)
				break;
		}
	}

	bool needs_update_each_second() const
	{
		return (_state == State::STARTING_UP) || (_state == State::SHUTTING_DOWN);
	}

	bool starting_up()   const { return _state == State::STARTING_UP; }
	bool shutting_down() const { return _state == State::SHUTTING_DOWN; }
	bool off()           const { return _state == State::OFF; }
	bool on()            const { return _state == State::ON; }
};

#endif /* _POWER_H_ */
