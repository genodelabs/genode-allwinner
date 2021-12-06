/*
 * \brief  Platform driver for Allwinner A64
 * \author Norman Feske
 * \date   2021-11-08
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/component.h>

#include <r_prcm.h>
#include <ccu.h>
#include <pmic.h>
#include <root.h>

namespace Driver { struct Main; };

struct Driver::Main
{
	void update_config();

	Env                  & env;
	Heap                   heap           { env.ram(), env.rm() };
	Sliced_heap            sliced_heap    { env.ram(), env.rm() };
	Attached_rom_dataspace config         { env, "config"       };
	Device_model           devices        { heap                };

	Fixed_clock osc_24m_clk { devices.clocks(), "osc24M",
	                          Clock::Rate { 24*1000*1000 } };
	Fixed_clock dummy_clk   { devices.clocks(), "dummy",
	                          Clock::Rate { 1000 } };

	R_prcm r_prcm { env, devices.clocks(), osc_24m_clk };
	Ccu    ccu    { env, devices.clocks(), devices.resets(), osc_24m_clk };
	Pmic   pmic   { env, devices.powers() };

	Signal_handler<Main>   config_handler { env.ep(), *this,
	                                      &Main::update_config };
	Driver::Root           root           { env, sliced_heap, config, devices };

	Main(Genode::Env & e)
	: env(e)
	{
		devices.update(config.xml());
		config.sigh(config_handler);
		env.parent().announce(env.ep().manage(root));
	}
};


void Driver::Main::update_config()
{
	config.update();
	devices.update(config.xml());
	root.update_policy();
}

void Component::construct(Genode::Env &env) {
	static Driver::Main main(env); }
