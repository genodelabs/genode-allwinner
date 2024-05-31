/*
 * \brief  Touchscreen driver
 * \author Norman Feske
 * \date   2021-09-24
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <util/reconstructible.h>
#include <lx_emul/init.h>
#include <lx_kit/env.h>
#include <lx_kit/init.h>
#include <genode_c_api/event.h>

namespace Touchscreen_driver {

	using namespace Genode;

	struct Main;
}


struct Touchscreen_driver::Main
{
	Env                   &_env;
	Attached_rom_dataspace _dtb_rom { _env, "dtb" };

	Signal_handler<Main> _signal_handler {
		_env.ep(), *this, &Main::_handle_signal };

	void _handle_signal()
	{
		Lx_kit::env().scheduler.execute();
	}

	Main(Env &env) : _env(env)
	{
		log("--- touchscreen driver started ---");

		Lx_kit::initialize(_env, _signal_handler);
		_env.exec_static_constructors();

		genode_event_init(genode_env_ptr(env),
		                  genode_allocator_ptr(Lx_kit::env().heap));

		lx_emul_start_kernel(_dtb_rom.local_addr<void>());
	}
};


void Component::construct(Genode::Env &env)
{
	static Touchscreen_driver::Main main(env);
}
