/*
 * \brief  LTE-modem manager
 * \author Sebastian Sumpf
 * \author Norman Feske
 * \date   2022-02-17
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <timer_session/connection.h>
#include <os/reporter.h>

/* AT-protocol driver */
#include <at_protocol/terminal_modem.h>
#include <at_protocol/driver.h>

/* local includes */
#include <power.h>

namespace Modem { struct Main; }


struct Modem::Main : private Delayer
{
	Env &_env;

	Attached_rom_dataspace _config { _env, "config" };

	Expanding_reporter _reporter { _env, "modem", "state" };

	bool _verbose = false;

	Timer::Connection _sleep_timer { _env };
	Timer::Connection _event_timer { _env };

	Power _power { _env, *this };

	uint64_t _startup_triggered_ms  = 0,  /* for startup-seconds counter */
	         _shutdown_triggered_ms = 0,  /* for shutdown_seconds counter */
	         _at_powered_down_ms    = 0,  /* for grace delay */
	         _last_at_progress_ms   = 0;  /* for handling command timeouts */

	enum class Power_state
	{
		UNKNOWN,
		POWERING_ON,         /* power driver in progress */
		AT_PROTOCOL,
		AT_BUSY,
		AT_SHUTDOWN,         /* AT driver in progress */
		AT_GRACE_DELAY,      /* wait 1 sec after AT shutdown */
		POWERING_OFF,        /* power driver in progress */
		POWER_OFF,
		POWER_ON,            /* entered only w/o AT driver */
	};

	Power_state _power_state { Power_state::UNKNOWN };

	struct At_protocol_driver_base
	{
		At_protocol::Terminal_modem modem;

		At_protocol_driver_base(Env &env) : modem(env) { }
	};

	struct At_protocol_driver : private At_protocol_driver_base,
	                            At_protocol::Driver<256>
	{
		At_protocol_driver(Env &env, Signal_context_capability sigh)
		: At_protocol_driver_base(env) { modem.sigh(sigh); };

		At_protocol::Qcfg::Entry _qcfg_usbnet_ecm { this->qcfg, "usbnet", "1" };

		void apply(Xml_node config)
		{
			Driver<256>::apply(config, modem, modem);
		}
	};

	Constructible<At_protocol_driver> _at_protocol_driver { };

	bool _at_response_outstanding() const
	{
		return _at_protocol_driver.constructed()
		    && _at_protocol_driver->response_outstanding();
	}

	bool _outbound_call() const
	{
		return _at_protocol_driver.constructed()
		    && _at_protocol_driver->outbound();
	}

	Signal_handler<Main> _config_handler {
		_env.ep(), *this, &Main::_handle_config };

	Signal_handler<Main> _timer_handler {
		_env.ep(), *this, &Main::_handle_timer };

	bool _timer_scheduled = false;

	void _trigger_timer_in_500ms()
	{
		if (!_timer_scheduled)
			_event_timer.trigger_once(500*1000UL);
		_timer_scheduled = true;
	}

	uint64_t _seconds_since(uint64_t back_then_ms) const
	{
		return (_event_timer.elapsed_ms() - back_then_ms) / 1000;
	}

	void _update_state_report()
	{
		_reporter.generate([&] (Xml_generator &xml) {
			_generate_report(xml); });
	}

	bool _starting_up() const
	{
		return (_power_state == Power_state::POWERING_ON);
	}

	bool _shutting_down() const
	{
		return (_power_state == Power_state::AT_SHUTDOWN)
		    || (_power_state == Power_state::AT_GRACE_DELAY)
		    || (_power_state == Power_state::POWERING_OFF);
	}

	bool _drive_at_protocol() const
	{
		return (_power_state == Power_state::AT_PROTOCOL)
		    || (_power_state == Power_state::AT_SHUTDOWN);
	}

	void _generate_report(Xml_generator &xml) const
	{
		auto attr_value = [] (Power_state state)
		{
			switch (state) {
			case Power_state::UNKNOWN:      break;
			case Power_state::POWERING_ON:  return "starting up";
			case Power_state::POWER_ON:
			case Power_state::AT_PROTOCOL:
			case Power_state::AT_BUSY:      return "on";
			case Power_state::AT_SHUTDOWN:
			case Power_state::AT_GRACE_DELAY:
			case Power_state::POWERING_OFF: return "shutting down";
			case Power_state::POWER_OFF:    return "off";
			};
			return "unknown";
		};

		if (_power_state != Power_state::UNKNOWN)
			xml.attribute("power", attr_value(_power_state));

		if (_starting_up())
			xml.attribute("startup_seconds", _seconds_since(_startup_triggered_ms));

		if (_shutting_down())
			xml.attribute("shutdown_seconds", _seconds_since(_shutdown_triggered_ms));

		if (_at_protocol_driver.constructed())
			_at_protocol_driver->generate_report(xml);
	}

	void _apply_power_state(Xml_node const &config)
	{
		bool const use_at_protocol = config.attribute_value("at_protocol", true);

		switch (_power_state) {

		case Power_state::POWER_OFF:
		case Power_state::UNKNOWN:

			if (config.attribute_value("power", false) == true) {

				_startup_triggered_ms = _event_timer.elapsed_ms();

				_power_state = Power_state::POWERING_ON;
			}
			break;

		case Power_state::POWER_ON:

			if (config.attribute_value("power", true) == false) {

				_shutdown_triggered_ms = _event_timer.elapsed_ms();

				_power_state = Power_state::POWERING_OFF;
			}
			break;

		case Power_state::POWERING_ON:

			_power.power_enabled(true); /* drive power-up sequence */

			if (_power.on()) {
				if (use_at_protocol) {
					_at_protocol_driver.construct(_env, _config_handler);
					_power_state = Power_state::AT_PROTOCOL;
					_at_protocol_driver->verbose = _verbose;
					_at_protocol_driver->apply(config);
				} else {
					_power_state = Power_state::POWER_ON;
				}
			}

			[[fallthrough]];  /* allow triggering power-off during startup */

		case Power_state::AT_PROTOCOL:
		case Power_state::AT_BUSY:

			if (config.attribute_value("power", true) == false) {

				_shutdown_triggered_ms = _event_timer.elapsed_ms();

				if (_at_protocol_driver.constructed()) {
					if (_at_protocol_driver->powering_down())
						_power_state = Power_state::AT_SHUTDOWN;

					if (_at_protocol_driver->powered_down())
						_power_state = Power_state::AT_SHUTDOWN;
				} else {
					_power_state = Power_state::POWERING_OFF;
				}
			}
			break;

		case Power_state::AT_SHUTDOWN:

			if (_at_protocol_driver.constructed()) {

				if (_at_protocol_driver->powered_down()) {
					_at_protocol_driver.destruct();
					_at_powered_down_ms = _event_timer.elapsed_ms();
					_power_state = Power_state::AT_GRACE_DELAY;
				}
			}

			/* give up after getting no response to the AT shutdown request */
			if (_seconds_since(_shutdown_triggered_ms) > 25) {
				warning("polite AT shutdown timed out");
				_power_state = Power_state::POWERING_OFF;
			}

			break;

		case Power_state::AT_GRACE_DELAY:

			if (_event_timer.elapsed_ms() - _at_powered_down_ms >= 1000)
				_power_state = Power_state::POWERING_OFF;
			break;

		case Power_state::POWERING_OFF:

			_power.power_enabled(false); /* drive power-down sequence */
			if (_power.off())
				_power_state = Power_state::POWER_OFF;
			break;
		}
	}

	void _handle_config()
	{
		_config.update();

		Xml_node const config = _config.xml();

		_verbose = config.attribute_value("verbose", false);

		if (_at_protocol_driver.constructed())
			_at_protocol_driver->verbose = _verbose;

		if (_verbose)
			log("config: ", config);

		bool overall_progress = false;

		using At_version = At_protocol::Status::Version;
		auto at_version = [&]
		{
			if (_at_protocol_driver.constructed())
				return _at_protocol_driver->status.version();

			return At_version { };
		};

		At_version const orig_at_version = at_version();

		auto busy_count = [&] {
			return _at_protocol_driver.constructed()
			     ? _at_protocol_driver->busy_count() : 0; };

		unsigned const orig_busy_count = busy_count();

		auto busy = [&] {
			return _at_protocol_driver.constructed()
			    && !_at_protocol_driver->powering_down()
			    && (orig_busy_count != busy_count()); };

		/*
		 * The state machines for the power driver and AT protocol driver
		 * are interdependent. Hence, we repeatedly execute both until
		 * their states settle.
		 */
		for (;;) {

			Power::State const loop_power_state = _power._state;
			At_version   const loop_at_version  = at_version();

			_apply_power_state(config);

			if (_at_protocol_driver.constructed() && _drive_at_protocol())
				_at_protocol_driver->apply(config);

			bool const progress = (loop_power_state      != _power._state)
			                   || (loop_at_version.value != at_version().value);
			if (!progress)
				break;

			overall_progress = true;

			if (busy())
				break;
		}

		if (busy())
			_power_state = Power_state::AT_BUSY;

		if (orig_at_version.value != at_version().value)
			_last_at_progress_ms = _event_timer.elapsed_ms();

		bool const need_polling = _starting_up()
		                       || _shutting_down()
		                       || _outbound_call()
		                       || (_power_state == Power_state::AT_BUSY)
		                       || _at_response_outstanding();

		if (need_polling)
			_trigger_timer_in_500ms();

		if (overall_progress || _starting_up() || _shutting_down())
			_update_state_report();
	}

	void _handle_timer()
	{
		_timer_scheduled = false;

		if (_power_state == Power_state::AT_BUSY)
			_power_state =  Power_state::AT_PROTOCOL;

		/* update call list while placing an outbound call */
		if (_outbound_call())
			_at_protocol_driver->invalidate_call_list();

		/* cancel timed-out command */
		if (_at_response_outstanding()) {
			uint64_t const duration_ms = _event_timer.elapsed_ms() - _last_at_progress_ms;
			if (duration_ms > _at_protocol_driver->command_timeout_ms())
				_at_protocol_driver->cancel_command();
		}

		_handle_config();
	}

	/**
	 * Delayer interface
	 */
	void msleep(unsigned long ms) override { _sleep_timer.msleep(ms); }

	Main(Env &env) : _env(env)
	{
		_config.sigh(_config_handler);
		_event_timer.sigh(_timer_handler);

		_handle_config();
	}
};


void Component::construct(Genode::Env &env)
{
	static Modem::Main main(env);
}

