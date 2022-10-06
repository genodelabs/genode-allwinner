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
#include <common.h>
#include <scp.h>

namespace Driver { struct Main; };

struct Driver::Main
{
	Env                  & _env;
	Attached_rom_dataspace _config_rom     { _env, "config"        };
	Common                 _common         { _env, _config_rom     };
	Signal_handler<Main>   _config_handler { _env.ep(), *this,
	                                         &Main::_handle_config };
	void _handle_config();

	Fixed_clock _osc_24m_clk { _common.devices().clocks(), "osc24M",
	                           Clock::Rate { 24*1000*1000 } };
	Fixed_clock _dummy_clk   { _common.devices().clocks(), "dummy",
	                           Clock::Rate { 1000 } };

	R_prcm _r_prcm { _env, _common.devices().clocks(), _osc_24m_clk };
	Ccu    _ccu    { _env, _common.devices().clocks(),
	                 _common.devices().resets(), _osc_24m_clk };
	Pmic   _pmic   { _env, _common.devices().powers() };

	Scp::Driver _scp { _env, _ccu._mbox_rst, _ccu._mbox_gate };

	Main(Genode::Env & env) : _env(env)
	{
		_config_rom.sigh(_config_handler);
		_handle_config();
		_common.announce_service();
		log("initialized");
	}
};


void Driver::Main::_handle_config()
{
	_config_rom.update();
	_common.handle_config(_config_rom.xml());
}


void Component::construct(Genode::Env &env) { static Driver::Main main(env); }
