/*
 * \brief  Driver for A64 power management IC
 * \author Norman Feske
 * \date   2021-11-08
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVERS__PLATFORM__A64__PMIC_H_
#define _SRC__DRIVERS__PLATFORM__A64__PMIC_H_

#include <os/attached_mmio.h>
#include <power.h>
#include <rsb.h>

namespace Driver { struct Pmic; }


struct Driver::Pmic : private Noncopyable
{
	Genode::Env &_env;
	Powers      &_powers;

	Rsb _rsb;

	struct Gpio_ldo : Power
	{
		Rsb &_rsb;

		Rsb::Reg const _reg;

		Gpio_ldo(Powers &powers, Power::Name const &name, Rsb &rsb, Rsb::Reg reg)
		:
			Power(powers, name), _rsb(rsb), _reg(reg)
		{ }

		void _on()  override { _rsb.write_byte(_reg, 3); }
		void _off() override { _rsb.write_byte(_reg, 7); }
	};

	Gpio_ldo gpio0_ldo { _powers, "pmic-gpio0", _rsb, Rsb::Reg { 0x90 } };

	Pmic(Genode::Env &env, Powers &powers)
	:
		_env(env), _powers(powers), _rsb(env)
	{ }
};

#endif /* _SRC__DRIVERS__PLATFORM__A64__PMIC_H_ */
