/*
 * \brief  LTE-modem manager
 * \author Sebastian Sumpf
 * \author Norman Feske
 * \date   2022-02-17
 *
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

/* local includes */
#include <audio_codec.h>
#include <power.h>

namespace Modem { struct Main; }


struct Modem::Main : private Delayer
{
	Env &_env;

	Attached_rom_dataspace _config { _env, "config" };

	Expanding_reporter _reporter { _env, "modem", "state" };

	Timer::Connection _timer { _env };

	Power _power { _env, *this };

	Signal_handler<Main> _config_handler {
		_env.ep(), *this, &Main::_handle_config };

	Signal_handler<Main> _timer_handler {
		_env.ep(), *this, &Main::_handle_timer };

	bool _timer_scheduled = false;

	void _trigger_timer_in_one_second()
	{
		_timer.trigger_once(1000*1000UL);
		_timer_scheduled = true;
	}

	void _update_state_report()
	{
		_reporter.generate([&] (Xml_generator &xml) {
			_generate_report(xml); });
	}

	void _generate_report(Xml_generator &xml) const
	{
		_power.generate_report(xml);
	}

	void _drive_state_transitions()
	{
		_power.drive_state_transitions();

		if (_power.needs_update_each_second() && !_timer_scheduled)
			_trigger_timer_in_one_second();

		_update_state_report();
	}

	void _handle_config()
	{
		_config.update();

		Xml_node const config = _config.xml();

		_power.apply_config(config);

		_drive_state_transitions();
	}

	void _handle_timer()
	{
		_timer_scheduled = false;
		_drive_state_transitions();
	}

	/**
	 * Delayer interface
	 */
	void msleep(unsigned long ms) override { _timer.msleep(ms); }

	struct Audio_driver
	{
		Platform::Connection _platform;
		Audio::Device device { _platform };
		Audio_driver(Env &env) : _platform(env) { }
	};

	Constructible<Audio_driver> _audio { };

	Main(Env &env) : _env(env)
	{
		_config.sigh(_config_handler);
		_timer.sigh(_timer_handler);
		_handle_config();

		_audio.construct(_env);
	}
};


void Component::construct(Genode::Env &env)
{
	static Modem::Main main(env);
}

