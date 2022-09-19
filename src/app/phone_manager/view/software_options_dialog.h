/*
 * \brief  Dialog for the software options
 * \author Norman Feske
 * \date   2022-09-20
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__SOFTWARE_OPTIONS_DIALOG_H_
#define _VIEW__SOFTWARE_OPTIONS_DIALOG_H_

#include <model/launchers.h>
#include <view/dialog.h>
#include <string.h>

namespace Sculpt { struct Software_options_dialog; }


struct Sculpt::Software_options_dialog
{
	Runtime_info const &_runtime_info;
	Launchers    const &_launchers;

	struct Action : Interface
	{
		virtual void enable_optional_component (Path const &launcher) = 0;
		virtual void disable_optional_component(Path const &launcher) = 0;
	};

	Action &_action;

	using Hover_result = Hoverable_item::Hover_result;

	Hoverable_item _item   { };
	Hoverable_item _switch { };

	Software_options_dialog(Runtime_info const &runtime_info,
	                        Launchers    const &launchers,
	                        Action             &action)
	:
		_runtime_info(runtime_info), _launchers(launchers), _action(action)
	{ }

	using Name = Path;

	void _gen_option(Xml_generator &xml, Name const &id, bool enabled) const
	{
		gen_named_node(xml, "frame", id, [&] {

			xml.attribute("style", "important");

			gen_named_node(xml, "float", "label", [&] {
				xml.attribute("west", "yes");
				gen_named_node(xml, "label", "label", [&] {
					xml.attribute("text", String<50>("  ", Pretty(id)));
					xml.attribute("min_ex", "15");
				});
			});

			gen_named_node(xml, "float", "onoff", [&] {
				xml.attribute("east", "yes");
				gen_named_node(xml, "hbox", "onoff", [&] {

					auto gen_option = [&] (auto id, bool selected, auto text)
					{
						gen_named_node(xml, "button", id, [&] {

							if (selected)
								xml.attribute("selected", "yes");

							xml.node("label", [&] {
								xml.attribute("text", text); });
						});
					};

					gen_option("off", !enabled, "  Off  ");
					gen_option("on",   enabled, "  On  ");
				});
			});
		});
	}

	void generate(Xml_generator &xml) const
	{
		gen_named_node(xml, "vbox", "options", [&] {
			_launchers.for_each([&] (Launchers::Info const &info) {
				_gen_option(xml, info.path,
				            _runtime_info.present_in_runtime(info.path)); }); });
	}

	Hover_result hover(Xml_node hover)
	{
		_item.match(hover, "vbox", "frame", "name");
		return _switch.match(hover, "vbox", "frame", "float", "hbox", "button", "name");
	}

	bool hovered() const { return _switch._hovered.valid(); }

	void click()
	{
		if (_switch._hovered == "on")
			_action.enable_optional_component(_item._hovered);

		if (_switch._hovered == "off")
			_action.disable_optional_component(_item._hovered);
	}

};

#endif /* _VIEW__SOFTWARE_OPTIONS_DIALOG_H_ */
