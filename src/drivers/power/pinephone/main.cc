/*
 * \brief  PinePhone power control and monitoring
 * \author Norman Feske
 * \date   2022-11-04
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <os/reporter.h>
#include <scp_session/connection.h>
#include <timer_session/connection.h>

namespace Power {

	using namespace Genode;

	struct Scp;
	struct Pmic_info;
	struct Main;
}


struct Power::Scp : Noncopyable
{
	::Scp::Connection _connection;

	Scp(Env &env) : _connection(env) { }

	using Response = String<128>;

	Response execute(char const *command)
	{
		Response response { };

		using Execute_error = ::Scp::Execute_error;

		_connection.execute(

			[&] (char *buf, size_t buf_len) {
				size_t const command_len = strlen(command);
				if (command_len > buf_len) {
					error("SCP command exceeds maximum request size");
					return 0UL;
				} else {
					copy_cstring(buf, command, buf_len);
					return command_len;
				}
			},

			[&] (char const *result, size_t len) {
				response = Response(Cstring(result, len));
			},

			[&] (Execute_error e) {
				switch (e) {
				case Execute_error::REQUEST_TOO_LARGE:
					error("unable to execute too large SCP request");
					break;
				case Execute_error::RESPONSE_TOO_LARGE:
					error("unable to retrieve too large SCP response");
					break;
				}
			}
		);
		return response;
	}
};


struct Power::Pmic_info
{
	bool     ac_present, battery_present, charging;
	float    voltage, charge_current, discharge_current;
	unsigned remaining_capacity;

	void generate(Xml_generator &xml) const
	{
		xml.attribute("voltage", voltage);

		if (ac_present)
			xml.attribute("ac_present", "yes");

		if (charging)
			xml.attribute("charging", "yes");

		if (battery_present) {
			xml.node("battery", [&] {

				xml.attribute("remaining_capacity", remaining_capacity);

				if (charging > 0)
					xml.attribute("charge_current", charge_current);

				if (discharge_current > 0) {
					xml.attribute("discharge_current", discharge_current);
					xml.attribute("power_draw", discharge_current*voltage);
				}
			});
		}
	}

	static Pmic_info from_scp(Scp &scp)
	{
		Scp::Response const response = scp.execute("power_info");

		using Token = Genode::Token<Scanner_policy_identifier_with_underline>;

		Token t { response.string() };

		unsigned numbers[6] { };

		for (unsigned &number : numbers) {
			t = t.eat_whitespace();
			ascii_to(t.start(), number);
			t = t.next();
		}

		/*
		 * The battery-present status (bit 5 in register 1) appears to be
		 * always set. To report stable information, check the pausibility
		 * of the remaining capacity and the measured charge current. On very
		 * low capacity, we'd expect to observe at least a small charge
		 * current.
		 */

		float    const charge_current     = (float)numbers[3] / 1000;
		unsigned const remaining_capacity = numbers[5];
		bool     const battery_broken     = (remaining_capacity <= 1)
		                                 && (charge_current < 0.1);
		bool     const battery_present    = ((numbers[1] & (1 << 5)) != 0);

		return Pmic_info {
			.ac_present         = (numbers[0] & (1 << 7)) != 0,
			.battery_present    = battery_present && !battery_broken,
			.charging           = (numbers[1] & (1 << 6)) != 0,
			.voltage            = (float)numbers[2] / 1000,
			.charge_current     = charge_current,
			.discharge_current  = (float)numbers[4] / 1000,
			.remaining_capacity = remaining_capacity
		};
	}
};


struct Power::Main
{
	Env &_env;

	Timer::Connection _timer { _env };

	Attached_rom_dataspace _config { _env, "config" };

	Scp _scp { _env };

	Expanding_reporter _reporter { _env, "power", "power" };

	Signal_handler<Main> _timer_handler {
		_env.ep(), *this, &Main::_handle_timer };

	void _gen_battery(Xml_generator &, Scp &) const;

	Pmic_info _pmic_info { };

	void _handle_timer()
	{
		Pmic_info orig_pmic_info = _pmic_info;

		_pmic_info = Pmic_info::from_scp(_scp);

		_reporter.generate([&] (Xml_generator &xml) {
			_pmic_info.generate(xml); });

		/*
		 * When connecting a charger, increase the charge current limit from
		 * the default 500 mA to 1500 mA (bits 4..7 in register 0x35) while
		 * keeping the temperature loop enabled (bit 3).
		 */
		if (!orig_pmic_info.ac_present && _pmic_info.ac_present)
			_scp.execute("38 35 pmic!");

		/*
		 * Whenever the charging state changes (inserting/removing the battery,
		 * or dis/connectiong the charger), re-enable the gauge to prevent
		 * register 0xb9 from reporting an outdated value.
		 */
		if (orig_pmic_info.charging != _pmic_info.charging)
			_scp.execute("0 b8 pmic!  c0 b8 pmic!");
	}

	Signal_handler<Main> _config_handler {
		_env.ep(), *this, &Main::_handle_config };

	void _handle_config()
	{
		_config.update();

		Xml_node const config = _config.xml();

		uint64_t const period_ms = config.attribute_value("period_ms", 5000UL);

		_timer.trigger_periodic(1000*period_ms);
	}

	Main(Env &env) : _env(env)
	{
		/*
		 * Configure TS pin function as external input (bit 2 in register 0x84).
		 * Otherwise, charging won't be activated when connecting A/C.
		 */
		_scp.execute("84 pmic@  4 or  84 pmic!");

		/*
		 * Increase V-hold setting to 4.5 V (bits 3..5 in register 0x30, keep
		 * default bit 0 set).
		 */
		_scp.execute("29 30 pmic!");

		_scp.execute(

			/* print decimal number */
			": .decimal  base @ decimal swap . base ! ; "

			/* print power information */
			": power_info "
			"  0  pmic@ .decimal "                 /* power-source stats */
			"  1  pmic@ .decimal "                 /* power mode and charger */
			"  78 pmic@ 10 * 79 pmic@ + .decimal " /* voltage */
			"  7a pmic@ 10 * 7b pmic@ + .decimal " /* charge current */
			"  7c pmic@ 10 * 7d pmic@ + .decimal " /* discharge current */
			"  b9 pmic@ 7f and .decimal "          /* battery capacity (percent) */
			"; "
		);

		/*
		 * Interactive debug utilities
		 */
		_scp.execute(": .pmicreg dup space . pmic@ dup .hex space .bits ;");
		_scp.execute(": .pmicregs  FOR AFT dup space .pmicreg cr 1 + THEN NEXT drop ;");
		_scp.execute(": o 0 pmic@ .bits space 1 pmic@ .bits ;");
		_scp.execute(": p power_info ;");

		_timer.sigh(_timer_handler);

		_config.sigh(_config_handler);
		_handle_config();
	}
};


void Component::construct(Genode::Env &env) { static Power::Main main(env); }
