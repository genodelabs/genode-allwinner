/*
 * \brief  Audio-control driver
 * \author Sebastian Sumpf
 * \author Norman Feske
 * \date   2022-08-12
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/component.h>

/* local includes */
#include <audio_codec.h>

namespace Audio_control { struct Main; }


struct Audio_control::Main
{
	Env &_env;

	Platform::Connection  _platform { _env };
	Audio_control::Device _device   { _platform };

	Attached_rom_dataspace _config { _env, "config" };

	Signal_handler<Main> _config_handler {
		_env.ep(), *this, &Main::_handle_config };

	void _handle_config()
	{
		_config.update();
		_device.apply_config(_config.xml());
	}

	Main(Env &env) : _env(env)
	{
		_config.sigh(_config_handler);
		_handle_config();
	}
};


void Component::construct(Genode::Env &env)
{
	static Audio_control::Main main(env);
}

