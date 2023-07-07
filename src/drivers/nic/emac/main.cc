/*
 * \brief  Allwinner EMAC Ethernet driver
 * \author Norman Feske
 * \date   2021-06-02
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <lx_kit/init.h>
#include <lx_kit/env.h>
#include <lx_emul/init.h>
#include <lx_emul/task.h>
#include <genode_c_api/uplink.h>

namespace Emac_driver {
	using namespace Genode;
	struct Main;
}


extern task_struct *user_task_struct_ptr;



struct Emac_driver::Main
{
	Env &_env;

	Attached_rom_dataspace _dtb { _env, "dtb" };

	/**
	 * Signal handler triggered by activity of the uplink connection
	 */
	Signal_handler<Main> _signal_handler { _env.ep(), *this, &Main::_handle_signal };

	void _handle_signal()
	{
		if (user_task_struct_ptr)
			lx_emul_task_unblock(user_task_struct_ptr);

		Lx_kit::env().scheduler.execute();

		genode_uplink_notify_peers();
	}

	Main(Env &env) : _env(env)
	{
		Lx_kit::initialize(env, _signal_handler);

		env.exec_static_constructors();

		genode_uplink_init(genode_env_ptr(env),
		                   genode_allocator_ptr(Lx_kit::env().heap),
		                   genode_signal_handler_ptr(_signal_handler));

		lx_emul_start_kernel(_dtb.local_addr<void>());
	}
};


void Component::construct(Genode::Env &env)
{
	static Emac_driver::Main main(env);
}
