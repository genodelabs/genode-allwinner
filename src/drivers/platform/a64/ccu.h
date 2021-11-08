/*
 * \brief  Driver for A64 CCU
 * \author Norman Feske
 * \date   2021-11-08
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVERS__PLATFORM__A64__CCU_H_
#define _SRC__DRIVERS__PLATFORM__A64__CCU_H_

#include <os/attached_mmio.h>
#include <clock.h>
#include <reset.h>

namespace Driver { struct Ccu; }


struct Driver::Ccu : private Attached_mmio
{
	Genode::Env &_env;
	Clocks      &_clocks;
	Resets      &_resets;
	Clock       &_osc_24m_clk;

	void *_regs() { return local_addr<void>(); }

	struct Gate : Clock
	{
		Clock &_parent;

		Gate(Clocks &clocks, Name const &name, Clock &parent)
		:
			Clock(clocks, name), _parent(parent)
		{ }

		void set_rate(unsigned long rate) override { _parent.set_rate(rate); }

		unsigned long get_rate() const override { return _parent.get_rate(); }

		void set_parent(Name) override { }
	};


	/*
	 * Bus clocks
	 */

	struct Bus_clk_gate : Gate, private Mmio
	{
		unsigned const _bit;

		enum { MASK = 0, PASS = 1 };

		struct Bus_clk_gating_reg : Register_array<0, 32, 32, 1> { };

		Bus_clk_gate(Clocks     &clocks,
		             Name const &name,
		             Clock      &parent,
		             void       *ccu_regs,
		             unsigned    reg_offset,
		             unsigned    bit)
		:
			Gate(clocks, name, parent), Mmio((addr_t)ccu_regs + reg_offset),
			_bit(bit)
		{ }

		void _enable()  override { write<Bus_clk_gating_reg>(PASS, _bit); }
		void _disable() override { write<Bus_clk_gating_reg>(MASK, _bit); }
	};

	Bus_clk_gate _i2s0_clk_gate { _clocks, "bus-i2s0", _osc_24m_clk, _regs(), 0x68, 12 };
	Bus_clk_gate _twi0_clk_gate { _clocks, "bus-twi0", _osc_24m_clk, _regs(), 0x6c, 0 };


	/*
	 * Reset domains
	 */

	struct Bus_soft_rst : Reset, private Mmio
	{
		unsigned const _bit;

		enum { ASSERT = 0, DEASSERT = 1 };

		struct Bus_soft_rst_reg : Register_array<0, 32, 32, 1> { };

		Bus_soft_rst(Resets &resets, Name const &name,
		             void *ccu_regs, unsigned reg_offset, unsigned bit)
		:
			Reset(resets, name), Mmio((addr_t)ccu_regs + reg_offset), _bit(bit)
		{ }

		void _deassert() override { write<Bus_soft_rst_reg>(DEASSERT, _bit); }
		void _assert()   override { write<Bus_soft_rst_reg>(ASSERT,   _bit); }
	};

	Bus_soft_rst _i2c0_soft_rst { _resets, "twi0", _regs(), 0x2d8, 0 };

	Ccu(Genode::Env &env, Clocks &clocks, Resets &resets, Clock &osc_24m_clk)
	:
		Attached_mmio(env, 0x1c20000, 0x400),
		_env(env), _clocks(clocks), _resets(resets), _osc_24m_clk(osc_24m_clk)
	{ }
};

#endif /* _SRC__DRIVERS__PLATFORM__A64__CCU_H_ */
