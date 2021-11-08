/*
 * \brief  Platform driver - Device abstraction for A64
 * \author Norman Feske
 * \date   2021-11-02
 *
 * Based in the i.MX plaform driver by Stefan Kalkowski
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <a64_device.h>
#include <clock.h>
#include <reset.h>
#include <session_component.h>


bool Driver::A64_device::acquire(Driver::Session_component & sc)
{
	bool const ret = Driver::Device::acquire(sc);

	if (!ret)
		return ret;

	_power_domain_list.for_each([&] (Power_domain & p) {

		bool ok = false;
		sc.env().powers.apply(p.name, [&] (Driver::Power &power) {
			power.on();
			ok = true;
		});

		if (!ok)
			warning("power domain ", p.name, " is unknown");
	});

	_reset_domain_list.for_each([&] (Reset_domain & r) {

		bool ok = false;
		sc.env().resets.apply(r.name, [&] (Driver::Reset &reset) {
			reset.deassert();
			ok = true;
		});

		if (!ok)
			warning("reset domain ", r.name, " is unknown");
	});

	_clock_list.for_each([&] (Clock &c) {

		bool ok = false;
		sc.env().clocks.apply(c.name, [&] (Driver::Clock &clock) {

			if (c.parent.valid())
				clock.set_parent(c.parent);

			if (c.rate)
				clock.set_rate(c.rate);

			clock.enable();
			ok = true;
		});

		if (!ok) {
			warning("clock ", c.name, " is unknown");
			return;
		}
	});

	sc.update_devices_rom();

	return ret;
}


void Driver::A64_device::release(Session_component & sc)
{
	return Driver::Device::release(sc);
}


void Driver::A64_device::_report_platform_specifics(Xml_generator     &xml,
                                                    Driver::Session_component &sc)
{
	_clock_list.for_each([&] (Clock &c) {
		sc.env().clocks.apply(c.name, [&] (Driver::Clock &clock) {
			xml.node("clock", [&] () {
				xml.attribute("rate", clock.get_rate());
				xml.attribute("name", c.driver_name); }); }); });

	_reset_domain_list.for_each([&] (Reset_domain &r) {
		sc.env().resets.apply(r.name, [&] (Driver::Reset &reset) {
			xml.node("reset-domain", [&] () {
				xml.attribute("name", reset.name); }); }); });
}
