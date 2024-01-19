/*
 * \brief  Driver for A64 R_PRCM device (power, reset, clock)
 * \author Norman Feske
 * \date   2021-11-08
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVERS__PLATFORM__A64__R_PRCM_H_
#define _SRC__DRIVERS__PLATFORM__A64__R_PRCM_H_

#include <os/attached_mmio.h>
#include <clock.h>

namespace Driver { struct R_prcm; }


struct Driver::R_prcm : private Attached_mmio<0>
{
	Genode::Env &_env;
	Clocks      &_clocks;
	Clock       &_osc_24m_clk;

	struct Gate : Clock
	{
		Clock &_parent;

		Gate(Clocks &clocks, Name const &name, Clock &parent)
		:
			Clock(clocks, name), _parent(parent)
		{ }

		void rate(Rate rate) override { _parent.rate(rate); }
		Rate rate() const    override { return _parent.rate(); }
	};

	template <unsigned BIT>
	struct Apb0_clk_gate : Gate, private Mmio<0x2c>
	{
		enum { MASK = 0, PASS = 1 };

		struct Gates : Register_array<0x28, 32, 32, 1> { };

		Apb0_clk_gate(Clocks               &clocks,
		              Name const           &name,
		              Clock                &parent,
		              Byte_range_ptr const &r_prcm_regs)
		:
			Gate(clocks, name, parent), Mmio(r_prcm_regs)
		{ }

		void _enable()  override { write<Gates>(PASS, BIT); }
		void _disable() override { write<Gates>(MASK, BIT); }
	};


	Apb0_clk_gate<6> _r_twi_clk { _clocks, "r_twi", _osc_24m_clk, Attached_mmio::range() };

	R_prcm(Genode::Env &env, Clocks &clocks, Clock &osc_24m_clk)
	:
		Attached_mmio(env, {(char *)0x1f01400, 0x100}),
		_env(env), _clocks(clocks), _osc_24m_clk(osc_24m_clk)
	{ }
};

#endif /* _SRC__DRIVERS__PLATFORM__A64__R_PRCM_H_ */
