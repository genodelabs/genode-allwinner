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
#include <scp.h>

namespace Driver { struct Pmic; }


struct Driver::Pmic : private Noncopyable
{
	Scp::Local_connection &_scp;

	Powers &_powers;

	struct Power : Driver::Power
	{
		Scp::Local_connection &_scp;

		/* noncopyable */
		Power(Power const &);
		void operator = (Power const &);

		char const *_scp_on;
		char const *_scp_off;

		Power(Powers &powers, Power::Name const &name, Scp::Local_connection &scp,
		      char const *scp_on, char const *scp_off)
		:
			Driver::Power(powers, name), _scp(scp), _scp_on(scp_on), _scp_off(scp_off)
		{ }

		void _on()  override { _scp.execute(_scp_on);  }
		void _off() override { _scp.execute(_scp_off); }
	};

	Power gpio0_ldo { _powers, "pmic-gpio0", _scp,
	                  "3 90 pmic!",
	                  "7 90 pmic!" };

	Pmic(Scp::Local_connection &scp, Powers &powers)
	:
		_scp(scp), _powers(powers)
	{ }
};

#endif /* _SRC__DRIVERS__PLATFORM__A64__PMIC_H_ */
