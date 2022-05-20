/*
 * \brief  Dialog for initiating calls
 * \author Norman Feske
 * \date   2022-06-29
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__OUTBOUND_DIALOG_H_
#define _VIEW__OUTBOUND_DIALOG_H_

#include <view/dialog.h>

namespace Sculpt { struct Outbound_dialog; }


struct Sculpt::Outbound_dialog
{
	Modem_state   const &_state;
	Current_call  const &_current_call;

	struct Action : Interface
	{
	};

	Action &_action;

	using Hover_result = Hoverable_item::Hover_result;

	Hoverable_item _choice   { };
	Hoverable_item _initiate { };

	bool _ready_to_call() const { return true; /* XXX depends on dialpad state */ }

	Outbound_dialog(Modem_state const &state, Current_call &current_call,
	                Action &action)
	:
		_state(state), _current_call(current_call), _action(action)
	{ }

	void _gen_outbound_choice(Xml_generator &xml) const
	{
		gen_named_node(xml, "frame", "outbound", [&] {

			xml.node("hbox", [&] {

				gen_named_node(xml, "button", "history", [&] {
					xml.attribute("style", "unimportant");

					xml.node("label", [&] {
						xml.attribute("text", "History");
					});
				});

				gen_named_node(xml, "button", "contacts", [&] {
					xml.attribute("style", "unimportant");

					xml.node("label", [&] {
						xml.attribute("text", "Contacts");
					});
				});

				gen_named_node(xml, "button", "dial", [&] {
					xml.attribute("selected", "yes");

					xml.node("label", [&] {
						xml.attribute("text", "Dial");
					});
				});
			});
		});
	}

	void generate(Xml_generator &xml) const
	{
		_gen_outbound_choice(xml);
	}

	Hover_result hover(Xml_node hover)
	{
		return _choice.match(hover, "frame", "hbox", "button", "name");
	}

	bool hovered() const { return _choice._hovered.valid(); }

	void click() { }
};

#endif /* _VIEW__OUTBOUND_DIALOG_H_ */
