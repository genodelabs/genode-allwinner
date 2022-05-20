/*
 * \brief  SIM PIN entry dialog
 * \author Norman Feske
 * \date   2022-05-20
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__PIN_DIALOG_H_
#define _VIEW__PIN_DIALOG_H_

#include <view/dialog.h>
#include <model/sim_pin.h>

namespace Sculpt { struct Pin_dialog; }


struct Sculpt::Pin_dialog
{
	using Hover_result = Hoverable_item::Hover_result;

	struct Action : Interface
	{
		virtual void append_sim_pin_digit(Sim_pin::Digit) = 0;
		virtual void remove_last_sim_pin_digit() = 0;
		virtual void confirm_sim_pin() = 0;
	};

	Blind_sim_pin const &_sim_pin;

	Action &_action;

	Hoverable_item _button { };

	Hoverable_item::Id _clicked { };

	Pin_dialog(Blind_sim_pin const &sim_pin, Action &action)
	: _sim_pin(sim_pin), _action(action) { }

	void generate(Xml_generator &xml) const
	{
		auto gen_spacer = [&] (auto id)
		{
			gen_named_node(xml, "label", id, [&] {
				xml.attribute("min_ex", 10);
				xml.attribute("text", "");
			});
		};

		gen_named_node(xml, "float", "pin", [&] {

			xml.node("frame", [&] {

				xml.attribute("style", "important");

				unsigned row_count = 0;

				struct Button { Hoverable_item::Id id; bool visible; };

				auto gen_row = [&] (Xml_generator &xml, Button const &left,
				                                        Button const &middle,
				                                        Button const &right)
				{
					auto gen_button = [&] (Xml_generator &xml, Button const &button)
					{
						gen_named_node(xml, "button", button.id, [&] {

							bool const touched = _button.hovered(_clicked);

							if (touched && button.visible && _button.hovered(button.id))
								xml.attribute("selected", "yes");

							if (!button.visible)
								xml.attribute("style", "invisible");

							xml.node("vbox", [&] {
								gen_spacer("above");
								xml.node("label", [&] {
									if (button.visible) {
										xml.attribute("text", button.id);
										xml.attribute("font", "title/regular");
									}
								});
								gen_spacer("below");
							});
						});
					};

					gen_named_node(xml, "hbox", String<10>(row_count), [&] {
						gen_button(xml, left);
						gen_button(xml, middle);
						gen_button(xml, right);
					});

					row_count++;
				};

				xml.node("vbox", [&] {
					gen_named_node(xml, "label", "hspacer", [&] {
						xml.attribute("min_ex", 20); });

					gen_spacer("above pin");

					gen_named_node(xml, "hbox", "pin", [&] {
						gen_named_node(xml, "label", "pin", [&] {
							xml.attribute("min_ex", 5);
							xml.attribute("text", " Enter SIM PIN ");
							xml.attribute("font", "title/regular");
						});
						gen_named_node(xml, "label", "entry", [&] {
							xml.attribute("min_ex", 10);
							xml.attribute("text", String<64>(" ", _sim_pin, " "));
							xml.attribute("font", "title/regular");
						});
					});

					gen_spacer("below pin");

					gen_row(xml, Button{"1", true}, Button{"2", true}, Button{"3", true});
					gen_row(xml, Button{"4", true}, Button{"5", true}, Button{"6", true});
					gen_row(xml, Button{"7", true}, Button{"8", true}, Button{"9", true});
					gen_row(xml, Button{"C", _sim_pin.at_least_one_digit()},
					             Button{"0", true},
					             Button{"OK", _sim_pin.suitable_for_unlock()});
				});
			});
		});
	}

	Hover_result hover(Xml_node hover)
	{
		return _button.match(hover, "float", "frame", "vbox", "hbox", "button", "name");
	}

	bool hovered() const { return _button._hovered.valid(); }

	void click()
	{
		_clicked = _button._hovered;

		for (unsigned i = 0; i < 10; i++)
			if (_button.hovered(i))
				_action.append_sim_pin_digit(Sim_pin::Digit{i});

		if (_button.hovered("C"))
			_action.remove_last_sim_pin_digit();

		if (_button.hovered("OK") && _sim_pin.suitable_for_unlock())
			_action.confirm_sim_pin();
	}

	void clack() { _clicked = Hoverable_item::Id(); }
};

#endif /* _VIEW__PIN_DIALOG_H_ */
