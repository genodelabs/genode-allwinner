/*
 * \brief  Allwinner A64 driver Linux USB port
 * \author Sebastian Sumpf
 * \date   2022-06-24
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <base/component.h>
#include <base/env.h>

#include <lx_emul/init.h>
#include <lx_emul/usb.h>
#include <lx_kit/env.h>
#include <lx_kit/init.h>
#include <lx_kit/initial_config.h>
#include <lx_user/io.h>

#include <genode_c_api/usb.h>

using namespace Genode;


struct Main : private Entrypoint::Io_progress_handler
{
	Env                  & env;
	Attached_rom_dataspace dtb_rom        { env, "dtb"           };
	Signal_handler<Main>   signal_handler { env.ep(), *this,
	                                        &Main::handle_signal };
	Sliced_heap            sliced_heap    { env.ram(), env.rm()  };

	/**
	 * Entrypoint::Io_progress_handler
	 */
	void handle_io_progress() override
	{
		genode_usb_notify_peers();
	}

	void handle_signal()
	{
		lx_user_handle_io();
		Lx_kit::env().scheduler.execute();
	}

	Main(Env & env) : env(env)
	{
		{
			Lx_kit::Initial_config config { env };
		}

		Lx_kit::initialize(env);
		env.exec_static_constructors();

		genode_usb_init(genode_env_ptr(env),
		                genode_allocator_ptr(sliced_heap),
		                genode_signal_handler_ptr(signal_handler),
		                &lx_emul_usb_rpc_callbacks);

		lx_emul_start_kernel(dtb_rom.local_addr<void>());

		env.ep().register_io_progress_handler(*this);
	}
};


void Component::construct(Env & env)
{
	static Main main(env);
}
