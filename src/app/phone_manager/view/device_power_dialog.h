/*
 * \brief  Device power-control dialog
 * \author Norman Feske
 * \date   2022-11-18
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__DEVICE_POWER_DIALOG_H_
#define _VIEW__DEVICE_POWER_DIALOG_H_

#include <util/formatted_output.h>
#include <view/dialog.h>
#include <model/power_state.h>

namespace Sculpt { struct Device_power_dialog; }


struct Sculpt::Device_power_dialog
{
	Power_state const &_power_state;

	Device_power_dialog(Power_state const &power_state)
	: _power_state(power_state) { }

	void _gen_battery_info(Xml_generator &xml) const
	{
		auto gen_value = [&] (auto label, double value, auto unit)
		{
			gen_named_node(xml, "label", "label", [&] {
				xml.attribute("text", label);
				xml.attribute("min_ex", "23");
			});

			auto pretty_value = [] (double value, auto unit)
			{
				using Text = String<64>;

				if (value < 1.0)
					return Text((unsigned)((value + 0.0005)*1000), " m", unit);

				unsigned const decimal   = (unsigned)(value + 0.005);
				unsigned const hundredth = (unsigned)(100*(value - decimal));

				return Text(decimal, ".",
				            Repeated(2 - printed_length(hundredth), "0"),
				            hundredth, " ", unit);
			};

			gen_named_node(xml, "label", "value", [&] {
				xml.attribute("min_ex", "8");
				xml.attribute("text", pretty_value(value, unit));
			});
		};

		gen_named_node(xml, "float", "battery", [&] {
			xml.node("frame", [&] {
				xml.attribute("style", "unimportant");
				xml.node("hbox", [&] {
					if (_power_state.charging)
						gen_value("   Battery charge current ",
						          _power_state.battery.charge_current, "A");
					else
						gen_value("   Battery power draw ",
						          _power_state.battery.power_draw, "W");
				});
			});
		});
	}

	void generate(Xml_generator &xml) const
	{
		gen_named_node(xml, "vbox", "vbox", [&] {

			if (_power_state.battery_present)
				_gen_battery_info(xml);
		});
	}
};

#endif /* _VIEW__DEVICE_POWER_DIALOG_H_ */
