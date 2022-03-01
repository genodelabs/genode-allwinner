/*
 * \brief  LTE-modem manager
 * \author Sebastian Sumpf
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
#include <base/log.h>
#include <pin_control_session/connection.h>
#include <pin_state_session/connection.h>
#include <timer_session/connection.h>

#include "audio_codec.h"

namespace Modem {

	using namespace Genode;
	struct Main;
}


struct Modem::Main
{
	Env &_env;

	Attached_rom_dataspace _config { _env, "config" };
	/* debugging, automatic power down after 60s */
	bool _power_down {
		_config.xml().attribute_value("power_down", false) };

	Timer::Connection _timer { _env };

	Pin_control::Connection _pin_battery    { _env, "battery" };
	Pin_control::Connection _pin_dtr        { _env, "dtr" };
	Pin_control::Connection _pin_enable     { _env, "enable" };
	Pin_control::Connection _pin_host_ready { _env, "host-ready" };
	Pin_control::Connection _pin_pwrkey     { _env, "pwrkey" };
	Pin_control::Connection _pin_reset      { _env, "reset" };

	Pin_state::Connection _pin_status  { _env, "status" };
	Pin_state::Connection _pin_ri      { _env, "ri" };

	Constructible<Audio::Device> _audio { };

	bool power_up()
	{
		_pin_battery.state(true);

		/* wait for power supply before pwrkey */
		_timer.msleep(30);

		/* modem already on try reboot */
		if (_pin_status.state() == 0  && power_down() == false)
			return false;

		_pin_reset.state(false);
		_pin_host_ready.state(false);
		_pin_enable.state(false); /* enable RF */
		_pin_dtr.state(false); /* no supsend */
		_timer.msleep(30);

		/* issue power key pulse >= 500ms */
		log("Power up modem ...");
		_pin_pwrkey.state(true);
		_timer.msleep(1000);
		_pin_pwrkey.state(false);

		/* wait for max 60s */
		for (unsigned i = 0; i < 60; i++) {
			log(i, "s: status: ", _pin_status.state() ? "off" : "on");
			if (_pin_status.state() == 0) return true;
			_timer.msleep(1000);
		}

		error("Power up modem: failed");
		return false;
	}

	bool power_down()
	{
		if (_pin_status.state()) return true;

		_pin_reset.state(true);
		_pin_enable.state(true);

		log ("Power down modem ...");
		_pin_pwrkey.state(true);
		_timer.msleep(1000);
		_pin_pwrkey.state(false);

		for (unsigned i = 0; i < 60; i++) {
			log(i, "s: status: ", _pin_status.state() ? "off" : "on");
			if (_pin_status.state() == 1) return true;
			_timer.msleep(1000);
		}

		error("Power down modem: failed");
		return false;
	}

	Main(Env &env) : _env(env)
	{
		if (!power_up()) {
			_pin_battery.state(false);
			return;
		}

		_audio.construct(_env);

		if (!_power_down) return;

		unsigned wait = 60;
		log("Power up modem: succeeded waiting ", wait, " s ...");
		_timer.msleep(wait * 1000);

		if (power_down() == false) {
			return;
		}

		log("Power down modem: succeeded");

		/* disable power supply */
		_pin_battery.state(false);
	}
};


void Component::construct(Genode::Env &env)
{
	static Modem::Main main(env);
}

