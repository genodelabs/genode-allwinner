/*
 * \brief  Dialog for the software tabs
 * \author Norman Feske
 * \date   2022-09-l9
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__SOFTWARE_TABS_DIALOG_H_
#define _VIEW__SOFTWARE_TABS_DIALOG_H_

#include <view/dialog.h>

namespace Sculpt { struct Software_tabs_dialog; }


struct Sculpt::Software_tabs_dialog
{
	using Hover_result = Hoverable_item::Hover_result;

	enum class Choice { RUNTIME, OPTIONS, STATUS };

	Choice _choice { Choice::RUNTIME };

	Hoverable_item _tab { };

	Choice _hovered_choice() const
	{
		if (_tab.hovered("runtime")) return Choice::RUNTIME;
		if (_tab.hovered("options")) return Choice::OPTIONS;
		if (_tab.hovered("status"))  return Choice::STATUS;
		return Choice::RUNTIME;
	}

	Software_tabs_dialog() { }

	void generate(Xml_generator &xml) const
	{
		gen_named_node(xml, "frame", "software_tabs", [&] {

				xml.node("hbox", [&] {

				auto gen_tab = [&] (auto id, Choice choice, auto label)
				{
					gen_named_node(xml, "button", id, [&] {
						if (_choice == choice)
							xml.attribute("selected", "yes");
						xml.node("label", [&] {
							xml.attribute("text", label); }); });
				};

				gen_tab("runtime", Choice::RUNTIME, "Runtime");
				gen_tab("options", Choice::OPTIONS, "Options");
				gen_tab("status",  Choice::STATUS,  "Status");
			});
		});
	}

	Hover_result hover(Xml_node hover)
	{
		return _tab.match(hover, "frame", "hbox", "button", "name");
	}

	bool hovered() const { return _tab._hovered.valid(); }

	void click() { _choice = _hovered_choice(); }

	bool runtime_selected() const { return _choice == Choice::RUNTIME; }
	bool options_selected() const { return _choice == Choice::OPTIONS; }
	bool status_selected()  const { return _choice == Choice::STATUS; }
};

#endif /* _VIEW__SOFTWARE_TABS_DIALOG_H_ */
