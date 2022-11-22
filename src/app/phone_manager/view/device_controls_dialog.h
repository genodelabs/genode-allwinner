/*
 * \brief  Device controls (e.g., brightness) dialog
 * \author Norman Feske
 * \date   2022-11-22
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__DEVICE_CONTROLS_DIALOG_H_
#define _VIEW__DEVICE_CONTROLS_DIALOG_H_

#include <view/dialog.h>
#include <view/hoverable_item.h>
#include <model/power_state.h>

namespace Sculpt { struct Device_controls_dialog; }


struct Sculpt::Device_controls_dialog
{
	Power_state const &_power_state;

	struct Action : Interface
	{
		virtual void select_brightness_level(unsigned) = 0;
	};

	Action &_action;

	Device_controls_dialog(Power_state const &power_state, Action &action)
	:
		_power_state(power_state), _action(action)
	{ }

	using Hover_result = Hoverable_item::Hover_result;

	using Id = Hoverable_item::Id;

	Hoverable_item _choice { };

	bool _dragged = false;

	void _gen_brightness(Xml_generator &xml) const
	{
		gen_named_node(xml, "frame", "brightness", [&] {

			xml.attribute("style", "important");

			gen_named_node(xml, "float", "label", [&] {
				xml.attribute("west", "yes");
				gen_named_node(xml, "label", "label", [&] {
					xml.attribute("text", "  Brightness");
					xml.attribute("min_ex", "15");
				});
			});

			gen_named_node(xml, "float", "onoff", [&] {
				xml.attribute("east", "yes");
				gen_named_node(xml, "hbox", "range", [&] {

					for (unsigned i = 0; i < 10; i++) {

						gen_named_node(xml, "button", Id(i), [&] {

							if (i*10 <= _power_state.brightness)
								xml.attribute("selected", "yes");
							else
								xml.attribute("style", "unimportant");

							xml.node("label", [&] {
								xml.attribute("text", " "); });
						});
					}
				});
			});
		});
	}

	void _select_brightness_level()
	{
		unsigned value = 0;
		if (ascii_to(_choice._hovered.string(), value))
			_action.select_brightness_level(value*10 + 9);
	}

	void generate(Xml_generator &xml) const
	{
		gen_named_node(xml, "vbox", "vbox", [&] {
			_gen_brightness(xml); });
	}

	Hover_result hover(Xml_node hover)
	{
		Hover_result const result =
			_choice.match(hover, "vbox", "frame", "float", "hbox", "button", "name");

		if (_dragged)
			_select_brightness_level();

		return result;
	}

	bool hovered() const { return _choice._hovered.valid(); }

	void click()
	{
		_dragged = true;
		_select_brightness_level();
	}

	void clack()
	{
		_dragged = false;
	}
};

#endif /* _VIEW__DEVICE_CONTROLS_DIALOG_H_ */
