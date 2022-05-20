/*
 * \brief  Modem power-control dialog
 * \author Norman Feske
 * \date   2022-05-20
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__MODEM_POWER_DIALOG_H_
#define _VIEW__MODEM_POWER_DIALOG_H_

#include <view/dialog.h>


namespace Sculpt { struct Modem_power_dialog; }


struct Sculpt::Modem_power_dialog
{
	Modem_state const &_state;

	struct Action : Interface
	{
		virtual void modem_power(bool) = 0;
	};

	Action &_action;

	using Hover_result = Hoverable_item::Hover_result;

	Hoverable_item _on_off { };

	Modem_power_dialog(Modem_state const &state, Action &action)
	: _state(state), _action(action) { }

	void generate(Xml_generator &xml) const
	{
		gen_named_node(xml, "frame", "modempower", [&] {

			xml.attribute("style", "important");

			gen_named_node(xml, "float", "label", [&] {
				xml.attribute("west", "yes");
				gen_named_node(xml, "label", "label", [&] {
					xml.attribute("text", "  Modem Power");
					xml.attribute("min_ex", "15");
				});
			});

			gen_named_node(xml, "float", "onoff", [&] {
				xml.attribute("east", "yes");
				gen_named_node(xml, "hbox", "onoff", [&] {

					auto gen_option = [&] (auto id, bool selected, auto text)
					{
						gen_named_node(xml, "button", id, [&] {

							if (_state.transient())
								xml.attribute("style", "unimportant");

							if (selected)
								xml.attribute("selected", "yes");

							xml.node("label", [&] {
								xml.attribute("text", text); });
						});
					};

					gen_option("off", !_state.on(), "  Off  ");
					gen_option("on",   _state.on(), "  On  ");
				});
			});
		});
	}

	Hover_result hover(Xml_node hover)
	{
		return _on_off.match(hover, "frame", "float", "hbox", "button", "name");
	}

	bool hovered() const { return _on_off._hovered.valid(); }

	void click()
	{
		if (_on_off.hovered("on"))  _action.modem_power(true);
		if (_on_off.hovered("off")) _action.modem_power(false);
	}
};

#endif /* _VIEW__MODEM_POWER_DIALOG_H_ */
