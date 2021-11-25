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

#ifndef _SRC__DRIVERS__PLATFORM__A64__ENV_H_
#define _SRC__DRIVERS__PLATFORM__A64__ENV_H_

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/env.h>
#include <base/heap.h>

/* platform-driver includes */
#include <device.h>
#include <r_prcm.h>
#include <ccu.h>
#include <pmic.h>

namespace Driver {

	using namespace Genode;

	struct Env;
};


struct Driver::Env
{
	Genode::Env           &env;
	Heap                   heap        { env.ram(), env.rm() };
	Sliced_heap            sliced_heap { env.ram(), env.rm() };
	Attached_rom_dataspace config      { env, "config"       };
	Device_model           devices     { *this               };
	Clocks                 clocks      { };
	Resets                 resets      { };
	Powers                 powers      { };

	Fixed_clock _osc_24m_clk { clocks, "osc24M", 24*1000*1000 };
	Fixed_clock _dummy_clk   { clocks, "dummy", 1000 };

	R_prcm r_prcm { env, clocks, _osc_24m_clk };
	Ccu    ccu    { env, clocks, resets, _osc_24m_clk };
	Pmic   pmic   { env, powers };

	Env(Genode::Env &env) : env(env) { }
};

#endif /* _SRC__DRIVERS__PLATFORM__A64__ENV_H_ */
