/*
 * \brief  Test for AT protocol handler
 * \author Norman Feske
 * \date   2022-06-15
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/component.h>
#include <base/attached_rom_dataspace.h>

/* local includes */
#include <modem.h>
#include <driver.h>

namespace Test {

	using namespace Genode;

	struct Main;
}


struct Test::Main
{
	Env &_env;

	Attached_rom_dataspace _config { _env, "config" };

	Constructible<Modem>  _modem  { };
	Constructible<Driver> _driver { };

	Main(Env &env) : _env(env)
	{
		using Role = String<16>;
		Role const role = _config.xml().attribute_value("role", Role());

		_modem .conditional(role == "modem",  _env);
		_driver.conditional(role == "driver", _env);
	}
};


void Component::construct(Genode::Env &env) { static Test::Main main(env); }
