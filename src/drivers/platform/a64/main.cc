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

	Scp::Driver _scp { _env, _ccu._mbox_rst, _ccu._mbox_gate };

	Pmic _pmic { _scp.local_connection, _common.devices().powers() };

	void _load_scp_firmware();

	Main(Genode::Env & env) : _env(env)
	{
		_config_rom.sigh(_config_handler);
		_handle_config();
		_load_scp_firmware();
		_common.announce_service();
	}
};


void Driver::Main::_handle_config()
{
	_config_rom.update();
	_common.handle_config(_config_rom.xml());
}


void Driver::Main::_load_scp_firmware()
{
	char const *firmware[] = {

		/* utilities */
		": /        0 swap um/mod swap drop ; "
		": invert   not ; "
		": lshift   FOR AFT dup + THEN NEXT ; "
		": rshift   FOR AFT 2 / THEN NEXT ; "
		": bitmask  1 swap lshift 1 - ; "
		": mdelay   1ab * FOR NEXT ; "
		": udelay   1 rshift FOR NEXT ;",

		/* bitfield access */
		": bf!  bitmask rot >r over lshift >r lshift r@ and r> invert r@ @ and or r> ! ;",
		": bf@  bitmask over lshift rot @ and swap rshift ;",
		": b!   1 bf! ;",
		": b@   1 bf@ ;",

		/* reduced serial bus (RSB) register addresses */
		": rsb 1f03400    ; "
		": ctrl  rsb      ; "
		": ccr   rsb 4  + ; "
		": stat  rsb c  + ; "
		": daddr rsb 10 + ; "
		": data  rsb 1c + ; "
		": pmcr  rsb 28 + ; "
		": cmd   rsb 2c + ; "
		": saddr rsb 30 + ; ",

		": rtsaddr.bf  saddr 10 8 ;",
		": sladdr.bf   saddr 0 10 ;",

		": wait_stat    BEGIN stat @ 1 = UNTIL 1 stat ! ;",
		": start_trans  1 ctrl 7 b!  wait_stat ;",

		/* enable prcm_r two-wire interface (TWI) */
		": prcm_r_twi 1f01400 28 + ;",
		"prcm_r_twi @ 40 or prcm_r_twi !",

		/* RSB reset (must be a complied word because of BEGIN/UNTIL) */
		": reset 1 ctrl 0 b!  BEGIN ctrl @ 0 = UNTIL ; reset",

		/* config CCR */
		"1 ccr 0 b!  1 ccr 1 b!  2 ccr 8 2 bf!",

		/* trigger PMU init sequence */
		"7c pmcr 10 8 bf!  1 pmcr 1f b!  wait_stat",

		/* set RSB slave */
		"e8 cmd !  2d rtsaddr.bf bf!  3a3 sladdr.bf bf!  start_trans",

		/* ( reg -- ) prepare PMIC read/write operation */
		": setup_rw  daddr !  2d rtsaddr.bf bf!  0 sladdr.bf bf! ;",

		/* ( reg -- value ) read PMIC register */
		": pmic@  setup_rw  8b cmd !  0 data !  start_trans data @ ;",

		/* ( value reg -- ) write PMIC register */
		": pmic!  setup_rw  4e cmd !  data !  start_trans ;",
	};

	for (char const *command : firmware) {
		auto response = _scp.local_connection.execute(command);

		if (response.length() > 1)
			log("SCP: ", response);
	}
}


void Component::construct(Genode::Env &env) { static Driver::Main main(env); }
