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



struct Emac_driver::Main : private Entrypoint::Io_progress_handler
{
	Env &_env;

	Attached_rom_dataspace _dtb { _env, "dtb" };

	/**
	 * Entrypoint::Io_progress_handler
	 */
	void handle_io_progress() override
	{
		genode_uplink_notify_peers();
	}

	/**
	 * Signal handler triggered by activity of the uplink connection
	 */
	Io_signal_handler<Main> _signal_handler { _env.ep(), *this, &Main::_handle_signal };

	unsigned _signal_handler_nesting_level = 0;

	void _handle_signal()
	{
		_signal_handler_nesting_level++;

		{
			if (!user_task_struct_ptr)
				return;

			lx_emul_task_unblock(user_task_struct_ptr);
			Lx_kit::env().scheduler.schedule();
		}

		/*
		 * Process currently pending I/O signals before leaving the signal
		 * handler to limit the rate of 'handle_io_progress' calls.
		 */
		if (_signal_handler_nesting_level == 1) {
			while (_env.ep().dispatch_pending_io_signal());
		}

		_signal_handler_nesting_level--;
	}

	Main(Env &env) : _env(env)
	{
		Lx_kit::initialize(env);

		env.exec_static_constructors();

		genode_uplink_init(genode_env_ptr(env),
		                   genode_allocator_ptr(Lx_kit::env().heap),
		                   genode_signal_handler_ptr(_signal_handler));

		lx_emul_start_kernel(_dtb.local_addr<void>());

		env.ep().register_io_progress_handler(*this);
	}
};


void Component::construct(Genode::Env &env)
{
	static Emac_driver::Main main(env);
}
