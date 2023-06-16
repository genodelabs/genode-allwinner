/*
 * \brief  i.MX8 SD-card driver Linux port
 * \author Stefan Kalkowski
 * \date   2021-06-29
 */

/*
 * Copyright (C) 2021-2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/env.h>

#include <lx_emul/init.h>
#include <lx_emul/shared_dma_buffer.h>
#include <lx_kit/env.h>
#include <lx_kit/init.h>
#include <lx_user/io.h>

#include <genode_c_api/block.h>

using namespace Genode;


struct Main : private Entrypoint::Io_progress_handler
{
	Env                  & env;
	Attached_rom_dataspace dtb_rom        { env, "dtb"           };
	Attached_rom_dataspace config         { env, "config"        };
	Signal_handler<Main>   config_handler { env.ep(), *this,
	                                        &Main::handle_config };
	Signal_handler<Main>   signal_handler { env.ep(), *this,
	                                        &Main::handle_signal };
	Sliced_heap            sliced_heap    { env.ram(), env.rm()  };

	/**
	 * Entrypoint::Io_progress_handler
	 */
	void handle_io_progress() override
	{
		genode_block_notify_peers();
	}

	void handle_config()
	{
		config.update();
		genode_block_apply_config(config.xml());
	}


	void handle_signal()
	{
		lx_user_handle_io();
		Lx_kit::env().scheduler.execute();
	}

	Main(Env & env) : env(env)
	{
		config.sigh(config_handler);

		Lx_kit::initialize(env);
		env.exec_static_constructors();

		genode_block_init(genode_env_ptr(env),
		                  genode_allocator_ptr(sliced_heap),
		                  genode_signal_handler_ptr(signal_handler),
		                  lx_emul_shared_dma_buffer_allocate,
		                  lx_emul_shared_dma_buffer_free);

		handle_config();

		lx_emul_start_kernel(dtb_rom.local_addr<void>());

		env.ep().register_io_progress_handler(*this);
	}
};


void Component::construct(Env & env) {
	static Main main(env); }
