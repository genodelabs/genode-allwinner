/*
 * \brief  Driver for A64 ruduced serial bus (RSB) interface
 * \author Norman Feske
 * \date   2021-11-19
 *
 * The RSB bus is a two-wire interconnect between the application processor and
 * the power-management chip (referred to as PMIC or AXP803). The register
 * interface of the RSB controller is described in the "A83T User Manual".
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__DRIVERS__PLATFORM__A64__RSB_H_
#define _SRC__DRIVERS__PLATFORM__A64__RSB_H_

#include <os/attached_mmio.h>

namespace Driver { struct Rsb; }


struct Driver::Rsb : private Attached_mmio
{
	private:

		struct Ctrl : Register<0x0, 32>
		{
			struct Start_trans : Bitfield<7, 1> { };
			struct Use_rsb     : Bitfield<8, 1> { };
			struct Soft_reset  : Bitfield<0, 1> { };
		};

		struct Ccr : Register<0x4, 32>
		{
			struct Trans_over_enb : Bitfield<0, 1> { };
			struct Trans_err_enb  : Bitfield<1, 1> { };
			struct Cd_odly        : Bitfield<8, 3> { };
		};

		struct Stat : Register<0xc, 32>
		{
			struct Tover : Bitfield<0, 1> { };
		};

		struct Daddr0 : Register<0x10, 32> { };

		struct Data0  : Register<0x1c, 32> { };

		struct Cmd : Register<0x2c, 32>
		{
			enum {
				RD8  = 0x8b, /* read one byte */
				WR8  = 0x4e, /* write one byte */
				SRTA = 0xe8, /* set run-time address */
			};
		};

		struct Pmcr : Register<0x28, 32>
		{
			struct Pmu_mode      : Bitfield<8,  8> { };
			struct Pmu_init_data : Bitfield<16, 8> { };
			struct Pmu_init_send : Bitfield<31, 1> { };
		};

		struct Saddr : Register<0x30, 32>
		{
			struct Rtsaddr : Bitfield<16, 8> { }; /* run-time slave address */
			struct Sladdr  : Bitfield<0, 16> { }; /* slave address */

			/*
			 * Magic values according to https://sunxi.org/RSB
			 */
			enum { RUNTIME_ADDR = 0x2d, HARDWARE_ADDR = 0x3a3 };
		};

		void _poll_and_clear_status()
		{
			while (read<Stat>() != 1) { }
			write<Stat>(1);
		}

	public:

		Rsb(Genode::Env &env) : Attached_mmio(env, 0x01f03400, 0x400)
		{
			write<Ctrl::Soft_reset>(1);
			while (read<Ctrl::Soft_reset>() == 1);

			/* configure clock control register */
			write<Ccr::Trans_over_enb>(1);
			write<Ccr::Trans_err_enb>(1);
			write<Ccr::Cd_odly>(2);

			/* trigger PMU init sequence to switch bus from TWI mode to RSB */
			write<Pmcr::Pmu_init_data>(0x7c);
			write<Pmcr::Pmu_init_send>(1);
			_poll_and_clear_status();

			/* set slave run-time address */
			write<Cmd>(Cmd::SRTA);
			write<Saddr::Rtsaddr>(Saddr::RUNTIME_ADDR);
			write<Saddr::Sladdr>(Saddr::HARDWARE_ADDR);
			write<Ctrl::Start_trans>(1);
			_poll_and_clear_status();
		};

		struct Reg { uint32_t value; };

		uint8_t read_byte(Reg reg)
		{
			write<Cmd>(Cmd::RD8);
			write<Daddr0>(reg.value);
			write<Saddr::Rtsaddr>(Saddr::RUNTIME_ADDR);
			write<Saddr::Sladdr>(0);
			write<Data0>(0);
			write<Ctrl::Start_trans>(1);
			_poll_and_clear_status();

			return read<Data0>() & 0xff;
		}

		void write_byte(Reg reg, uint8_t value)
		{
			write<Cmd>(Cmd::WR8);
			write<Daddr0>(reg.value);
			write<Saddr::Rtsaddr>(Saddr::RUNTIME_ADDR);
			write<Saddr::Sladdr>(0);
			write<Data0>(value);
			write<Ctrl::Start_trans>(1);
			_poll_and_clear_status();
		}
};

#endif /* _SRC__DRIVERS__PLATFORM__A64__RSB_H_ */
