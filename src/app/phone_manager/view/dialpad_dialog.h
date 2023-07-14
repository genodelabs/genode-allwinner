/*
 * \brief  Dialpad dialog
 * \author Norman Feske
 * \date   2022-06-29
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__DIALPAD_DIALOG_H_
#define _VIEW__DIALPAD_DIALOG_H_

#include <view/dialog.h>
#include <model/dialed_number.h>

namespace Sculpt { struct Dialpad_dialog; }


struct Sculpt::Dialpad_dialog
{
	Dialed_number const &_dialed_number;

	struct Action : Interface
	{
		virtual void append_dial_digit(Dialed_number::Digit) = 0;
	};

	Action &_action;

	using Hover_result = Hoverable_item::Hover_result;

	Hoverable_item _button { };

	Hoverable_item::Id _clicked { };

	Dialpad_dialog(Dialed_number const &dialed_number, Action &action)
	:
		_dialed_number(dialed_number), _action(action)
	{ }

	void generate(Xml_generator &xml) const
	{
		auto gen_spacer = [&] (auto id)
		{
			gen_named_node(xml, "label", id, [&] {
				xml.attribute("min_ex", 10);
				xml.attribute("text", "");
			});
		};

		gen_named_node(xml, "float", "dialpad", [&] {

			xml.node("frame", [&] {

				xml.attribute("style", "important");

				unsigned row_count = 0;

				struct Button { Hoverable_item::Id id; };

				auto gen_row = [&] (Xml_generator &xml, Button const &left,
				                                        Button const &middle,
				                                        Button const &right)
				{
					auto gen_button = [&] (Xml_generator &xml, Button const &button)
					{
						gen_named_node(xml, "button", button.id, [&] {

							bool const touched = _button.hovered(_clicked);

							if (touched && _button.hovered(button.id))
								xml.attribute("selected", "yes");

							xml.node("vbox", [&] {
								gen_spacer("above");
								xml.node("label", [&] {
									xml.attribute("text", button.id);
									xml.attribute("font", "title/regular");
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

					gen_spacer("above");

					gen_named_node(xml, "hbox", "number", [&] {
						gen_named_node(xml, "label", "number", [&] {
							xml.attribute("min_ex", 5);
							xml.attribute("text", " Dial ");
							xml.attribute("font", "title/regular");
						});
						gen_named_node(xml, "label", "entry", [&] {
							xml.attribute("min_ex", 10);
							xml.attribute("text", String<64>(" ", _dialed_number, " "));
							xml.attribute("font", "title/regular");
						});
					});

					gen_spacer("below");

					gen_row(xml, Button{"1"}, Button{"2"}, Button{"3"});
					gen_row(xml, Button{"4"}, Button{"5"}, Button{"6"});
					gen_row(xml, Button{"7"}, Button{"8"}, Button{"9"});
					gen_row(xml, Button{"*"}, Button{"0"}, Button{"#"});
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

		Dialed_number::Digit const digit { _button._hovered.string()[0] };

		_action.append_dial_digit(digit);
	}

	void clack() { _clicked = Hoverable_item::Id(); }
};

#endif /* _VIEW__DIALPAD_DIALOG_H_ */
