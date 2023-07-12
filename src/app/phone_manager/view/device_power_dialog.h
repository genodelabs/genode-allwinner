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
#include <view/hoverable_item.h>
#include <view/activatable_item.h>
#include <model/power_state.h>

namespace Sculpt { struct Device_power_dialog; }


struct Sculpt::Device_power_dialog
{
	Power_state const &_power_state;

	struct Action : Interface
	{
		virtual void activate_performance_power_profile() = 0;
		virtual void activate_economic_power_profile() = 0;
		virtual void trigger_device_reboot() = 0;
		virtual void trigger_device_off() = 0;
	};

	Action &_action;

	Device_power_dialog(Power_state const &power_state, Action &action)
	:
		_power_state(power_state), _action(action)
	{ }

	using Hover_result = Hoverable_item::Hover_result;

	using Id   = Hoverable_item::Id;
	using Text = String<64>;

	Id _selected  { };
	Id _confirmed { };

	Hoverable_item   _choice  { };
	Activatable_item _confirm { };

	bool _power_profile_active(Id const &id) const
	{
		switch (_power_state.profile) {
		case Power_state::Profile::PERFORMANCE: return (id == "performance");
		case Power_state::Profile::ECONOMIC:    return (id == "economic");
		case Power_state::Profile::UNKNOWN:     break;
		}
		return false;
	}

	struct Entry_attr
	{
		bool need_confirmation;
	};

	void _gen_horizontal_spacer(Xml_generator &xml) const
	{
		gen_named_node(xml, "label", "spacer", [&] {
			xml.attribute("min_ex", 35); });
	}

	void _gen_entry(Xml_generator &xml, Id const &id, Text const &text,
	                Entry_attr attr) const
	{
		bool const any_selected = _selected.length() > 1;

		gen_named_node(xml, "hbox", id, [&] () {

			gen_named_node(xml, "float", "left", [&] () {
				xml.attribute("west", "yes");

				xml.node("hbox", [&] () {
					gen_named_node(xml, "float", "radio", [&] () {
						gen_named_node(xml, "button", "button", [&] () {

							if (_selected == id || (!any_selected && _power_profile_active(id)))
								xml.attribute("selected", "yes");

							xml.attribute("style", "radio");

							xml.node("hbox", [&] () { });
						});
					});
					gen_named_node(xml, "label", "name", [&] () {
						xml.attribute("text", Text(" ", text)); });
				});
			});

			gen_named_node(xml, "hbox", "middle", [&] () { });

			bool const ask_confirm = attr.need_confirmation && (_selected == id);

			gen_named_node(xml, "float", "right", [&] () {
				xml.attribute("east", "yes");

				gen_named_node(xml, "hbox", "right", [&] () {

					gen_named_node(xml, "button", id, [&] () {

						if (ask_confirm)
							_confirm.gen_button_attr(xml, id);
						else
							xml.attribute("style", "invisible");

						gen_named_node(xml, "label", "name", [&] () {
							xml.attribute("text", "Confirm");
							if (!ask_confirm)
								xml.attribute("style", "invisible");
						});
					});
				});
			});
		});
	}

	void _gen_power_options(Xml_generator &xml) const
	{
		gen_named_node(xml, "float", "options", [&] {
			xml.node("frame", [&] {
				xml.node("vbox", [&] {
					_gen_entry(xml, "performance", "Performance", { .need_confirmation = false });
					_gen_entry(xml, "economic",    "Economic",    { .need_confirmation = false });
					_gen_entry(xml, "reboot",      "Reboot",      { .need_confirmation = true });
					_gen_entry(xml, "off",         "Power down",  { .need_confirmation = true });

					_gen_horizontal_spacer(xml);
				});
			});
		});
	}

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
				xml.node("vbox", [&] {
					xml.node("hbox", [&] {
						if (_power_state.charging)
							gen_value("   Battery charge current ",
							          _power_state.battery.charge_current, "A");
						else
							gen_value("   Battery power draw ",
							          _power_state.battery.power_draw, "W");
					});
					_gen_horizontal_spacer(xml);
				});
			});
		});
	}

	void generate(Xml_generator &xml) const
	{
		gen_named_node(xml, "vbox", "vbox", [&] {

			_gen_power_options(xml);

			if (_power_state.battery_present)
				_gen_battery_info(xml);
		});
	}

	Hover_result hover(Xml_node hover)
	{
		return Deprecated_dialog::any_hover_changed(
			_choice .match(hover, "vbox", "float", "frame", "vbox", "hbox", "name"),
			_confirm.match(hover, "vbox", "float", "frame", "vbox", "hbox",
			                      "float", "hbox", "button", "name")
		);
	}

	bool hovered() const { return _choice._hovered.valid(); }

	void click()
	{
		if (_confirm.hovered(_selected))
			_confirm.propose_activation_on_click();

		if (_choice._hovered.valid())
			_selected = _choice._hovered;

		if (_choice.hovered("performance"))
			_action.activate_performance_power_profile();

		if (_choice.hovered("economic"))
			_action.activate_economic_power_profile();
	}

	void clack()
	{
		_confirm.confirm_activation_on_clack();

		if (_confirm.activated("reboot"))
			_action.trigger_device_reboot();

		if (_confirm.activated("off"))
			_action.trigger_device_off();

		_confirm.reset();
	}
};

#endif /* _VIEW__DEVICE_POWER_DIALOG_H_ */
