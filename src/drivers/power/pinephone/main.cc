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
#include <input/keycodes.h>
#include <os/reporter.h>
#include <platform_session/device.h>
#include <event_session/connection.h>
#include <scp_session/connection.h>
#include <timer_session/connection.h>

namespace Power {

	using namespace Genode;

	struct Scp;
	struct Pmic_info;
	struct Pmic;
	struct Rintc;
	struct Main;

	using Token = Genode::Token<Scanner_policy_identifier_with_underline>;
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


/**
 * Utility to access PMIC registers via Genode's 'Register' framework
 */
struct Power::Pmic : Register_set<Pmic>
{
	Scp &_scp;

	struct Scp_hex : Hex
	{
		template <typename T>
		explicit Scp_hex(T value) : Hex(value, OMIT_PREFIX) { }
	};

	using Request = String<64>;

	template <typename T>
	void _write(off_t const offset, T const value)
	{
		Request const request { Scp_hex(value), " ", Scp_hex(offset), " pmic!" };

		_scp.execute(request.string());
	}

	template <typename T>
	T _read(off_t const &offset) const
	{
		Request const request { Scp_hex(offset), " pmic@ ." };

		Scp::Response const response = _scp.execute(request.string());

		Token t { response.string() };
		t = t.eat_whitespace();

		uint8_t value = 0;
		ascii_to_unsigned(t.start(), value, 16);
		return value;
	}

	struct Irq_status_5 : Register<0x4c, 8>
	{
		struct Poksirq : Bitfield<4, 1> { };  /* short press on power button */
	};

	Pmic(Scp &scp) : Register_set<Pmic>(*this), _scp(scp) { }
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


/*
 * Nested interrupt controller that forwards the PMIC's IRQ (ENMI) to the GIC's
 * interrupt 64.
 */
struct Power::Rintc
{
	struct Mmio : Platform::Device::Mmio
	{
		struct Ctrl    : Register<0x0c, 32> { struct Pmic : Bitfield<0, 1> { }; };
		struct Pending : Register<0x10, 32> { struct Pmic : Bitfield<0, 1> { }; };
		struct Enable  : Register<0x40, 32> { struct Pmic : Bitfield<0, 1> { }; };
		struct Mask    : Register<0x50, 32> { struct Pmic : Bitfield<0, 1> { }; };

		using Platform::Device::Mmio::Mmio;

	} _mmio;

	Platform::Device::Irq  _irq;

	Rintc(Platform::Device &device, Signal_context_capability sigh)
	: _mmio(device), _irq(device) { _irq.sigh(sigh); }

	void ack()
	{
		/* clear interrupt at rintc */
		_mmio.write<Mmio::Enable::Pmic>(1);
		_mmio.write<Mmio::Pending::Pmic>(1);

		/* clear interrupt at GIC */
		_irq.ack();
	}
};


struct Power::Main
{
	Env &_env;

	Event::Connection _event { _env }; /* for reporting the power button */

	Timer::Connection _timer { _env };

	Attached_rom_dataspace _config { _env, "config" };

	using System_rom_label = String<64>;
	using Power_profile    = String<32>;

	uint64_t         _period_ms = 0;
	System_rom_label _system_rom_label { };
	Power_profile    _power_profile    { };
	unsigned         _brightness       { };

	Constructible<Attached_rom_dataspace> _system { };

	Scp _scp { _env };

	Expanding_reporter _reporter { _env, "power", "power" };

	Signal_handler<Main> _timer_handler {
		_env.ep(), *this, &Main::_handle_timer };

	void _gen_battery(Xml_generator &, Scp &) const;

	Pmic_info _pmic_info { };

	Pmic _pmic { _scp };

	Platform::Connection _platform { _env };

	Platform::Device _rintc_device { _platform };

	Signal_handler<Main> _rintc_handler {
		_env.ep(), *this, &Main::_handle_rintc };

	Rintc _rintc { _rintc_device, _rintc_handler };

	void _handle_rintc()
	{
		if (_pmic.read<Pmic::Irq_status_5::Poksirq>()) {

			_event.with_batch([&] (Event::Session_client::Batch &batch) {
				batch.submit( Input::Press   { Input::KEY_POWER });
				batch.submit( Input::Release { Input::KEY_POWER });
			});

			uint8_t value = 0;
			Pmic::Irq_status_5::Poksirq::set(value, 1);
			_pmic.write<Pmic::Irq_status_5>(value);
		}

		_scp.execute("clear_pmic_irqs");

		_rintc.ack();

		/*
		 * The PMIC interrupt triggers whenever something interesting happens,
		 * e.g., when connecting AC or when inserting the battery. Immediately
		 * reflect the new status in the power report.
		 */
		_update_power_report();
	}

	void _update_power_report()
	{
		auto brightness_from_scp = [&]
		{
			Scp::Response const response = _scp.execute("rpwmphase bf@ .decimal");

			unsigned result = 0;

			/* parse number following the leading whitespace of the response */
			Token t { response.string() };
			t = t.eat_whitespace();
			ascii_to(t.start(), result);

			/* scale value range from (0...1023) to (0...99) */
			return (result*100)/1024;
		};

		_reporter.generate([&] (Xml_generator &xml) {

			xml.attribute("brightness", brightness_from_scp());

			if (_power_profile.length() > 1)
				xml.attribute("power_profile", _power_profile);

			_pmic_info.generate(xml);
		});
	}

	void _handle_timer()
	{
		Pmic_info orig_pmic_info = _pmic_info;

		_pmic_info = Pmic_info::from_scp(_scp);

		_update_power_report();

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

	void _apply_system(Xml_node const &system)
	{
		using State = String<32>;
		State const state = system.attribute_value("state", State());

		if (state == "reset") {

			/* set bit 6 (soft power restart) in PMIC register 0x31 */
			_scp.execute("31 dup pmic@  40 or  swap pmic!");
		}

		if (state == "poweroff") {

			/* set bit 7 (power disable control) in PMIC register 0x32 */
			_scp.execute("32 dup pmic@  80 or  swap pmic!");
		}

		Power_profile const orig_power_profile = _power_profile;
		_power_profile = system.attribute_value("power_profile", Power_profile());

		if (orig_power_profile != _power_profile) {

			/* 1296 MHz with increased DCDC2/3 voltages (PMIC registers 0x21, 0x22) */
			if (_power_profile == "performance") {
				_scp.execute("70 dup 21 pmic! 22 pmic! "
				             "40 udelay "
				             "1a pllcpuxn bf! "
				             "40 udelay"
				);
			}

			/* 816 MHz at reduced DCDC2/3 voltages */
			if (_power_profile == "economic") {
				_scp.execute("10 pllcpuxn bf! "
				             "40 udelay "
				             "3c dup 21 pmic! 22 pmic! "
				             "40 udelay "
				             "30 dup 21 pmic! 22 pmic! "
				             "40 udelay"
				);
			}
		}

		/* apply brightness setting */
		unsigned const orig_brightness = _brightness;
		_brightness = system.attribute_value("brightness", 0u);
		if (orig_brightness != _brightness) {
			unsigned const pwm_phase = (_brightness*1023)/100;
			String<128> const
				command(Hex(pwm_phase, Hex::OMIT_PREFIX), " 1f03804 0 10 bf!");
			_scp.execute(command.string());

			/* reflect change immediately */
			_update_power_report();
		}
	}

	void _handle_config()
	{
		_config.update();

		Xml_node const config = _config.xml();

		/*
		 * Request system ROM depending on the configured 'system_rom' label
		 */
		System_rom_label const orig_system_rom_label = _system_rom_label;
		_system_rom_label = config.attribute_value("system_rom", System_rom_label());

		if (orig_system_rom_label != _system_rom_label) {
			if (_system_rom_label.length() > 1) {
				_system.construct(_env, _system_rom_label.string());
				_system->sigh(_config_handler);
			} else {
				_system.destruct();
			}
		}

		if (_system.constructed()) {
			_system->update();
			_apply_system(_system->xml());
		}

		uint64_t const orig_period_ms = _period_ms;
		_period_ms = config.attribute_value("period_ms", 5000UL);
		if (orig_period_ms != _period_ms)
			_timer.trigger_periodic(1000*_period_ms);
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

			/*
			 * bit field for the active phase in the R_PWM device
			 *
			 * The PWM for the backlight is configured for a total phase of
			 * 1024 cycles. Hence the value for the active phase must be
			 * in the range of 0...1023.
			 */
			": rpwmphase  1f03804 0 10 ; "

			/* bit field for factor N in PLL-CPUX control register */
			": pllcpuxn 1c20000 8 4 ; "

			/* print power information */
			": power_info "
			"  0  pmic@ .decimal "                 /* power-source stats */
			"  1  pmic@ .decimal "                 /* power mode and charger */
			"  78 pmic@ 10 * 79 pmic@ + .decimal " /* voltage */
			"  7a pmic@ 10 * 7b pmic@ + .decimal " /* charge current */
			"  7c pmic@ 10 * 7d pmic@ + .decimal " /* discharge current */
			"  b9 pmic@ 7f and .decimal "          /* battery capacity (percent) */
			"; "

			/* clear all PMIC interrupt status bits */
			": clear_pmic_irqs 48 4 FOR dup ff swap pmic! 1 + NEXT drop ; "
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
