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

	Power csi_power { _powers, "pmic-csi", _scp,
	                  /* ALDO1, set bit 5 in output control 3 */
	                  "b 28 pmic! "
	                  "13 pmic@ 20 or 13 pmic! "
	                  "32 udelay "
	                  /* ELDO3, set bit 2 in output control 2 */
	                  "10 1b pmic! "
	                  "12 pmic@ 4 or 12 pmic! "
	                  "a udelay "
	                  /* DLDO3, set bit 5 in output control 2 */
	                  "15 17 pmic! "
	                  "12 pmic@ 20 or 12 pmic! "
	                  "a udelay "
	                  ,
	                  /* DLDO3, clear bit 5 in output control 2 */
	                  "12 pmic@ df and 12 pmic! "
	                  "15 17 pmic! "
	                  /* ELDO3, clear bit 2 in output control 2 */
	                  "12 pmic@ fb and 12 pmic! "
	                  "0 1b pmic! "
	                  /* ALDO1, clear bit 5 in output control 3 */
	                  "13 pmic@ df and 13 pmic! "
	                  "17 28 pmic! "
	};

	Power wifi_power { _powers, "pmic-wifi", _scp,
	                  /* DLDO4, set bit 6 in output control 2 */
	                  "b 18 pmic! "
	                  "12 pmic@ 40 or 12 pmic! "
	                  "a udelay "
	                  ,
	                  /* DLDO4, clear bit 6 in output control 2 */
	                  "12 pmic@ bf and 12 pmic! "
	                  "1a 18 pmic! "
	};

	Pmic(Scp::Local_connection &scp, Powers &powers)
	:
		_scp(scp), _powers(powers)
	{ }
};

#endif /* _SRC__DRIVERS__PLATFORM__A64__PMIC_H_ */
