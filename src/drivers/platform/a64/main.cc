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
	Env                  & _env;
	Heap                   _heap        { _env.ram(), _env.rm() };
	Sliced_heap            _sliced_heap { _env.ram(), _env.rm() };
	Attached_rom_dataspace _config      { _env, "config" };
	Device_model           _devices     { _heap };

	Fixed_clock _osc_24m_clk { _devices.clocks(), "osc24M",
	                           Clock::Rate { 24*1000*1000 } };
	Fixed_clock _dummy_clk   { _devices.clocks(), "dummy",
	                           Clock::Rate { 1000 } };

	R_prcm _r_prcm { _env, _devices.clocks(), _osc_24m_clk };
	Ccu    _ccu    { _env, _devices.clocks(), _devices.resets(), _osc_24m_clk };
	Pmic   _pmic   { _env, _devices.powers() };

	void _handle_config();

	Signal_handler<Main> _config_handler { _env.ep(), *this,
	                                       &Main::_handle_config };

	Driver::Root _root { _env, _sliced_heap, _config, _devices };

	Main(Genode::Env &env) : _env(env)
	{
		_devices.update(_config.xml());
		_config.sigh(_config_handler);
		_env.parent().announce(_env.ep().manage(_root));
	}
};


void Driver::Main::_handle_config()
{
	_config.update();
	_devices.update(_config.xml());
	_root.update_policy();
}


void Component::construct(Genode::Env &env) { static Driver::Main main(env); }
