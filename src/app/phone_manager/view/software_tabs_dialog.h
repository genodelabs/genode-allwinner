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
#include <model/presets.h>

namespace Sculpt { struct Software_tabs_dialog; }


struct Sculpt::Software_tabs_dialog
{
	using Hover_result = Hoverable_item::Hover_result;

	Storage_target  const &_storage_target;
	Presets         const &_presets;
	Software_status const &_status;

	enum class Choice { PRESETS, RUNTIME, OPTIONS, UPDATE, STATUS };

	Choice _choice { Choice::RUNTIME };

	Hoverable_item _tab { };

	Choice _hovered_choice() const
	{
		if (_tab.hovered("presets")) return Choice::PRESETS;
		if (_tab.hovered("runtime")) return Choice::RUNTIME;
		if (_tab.hovered("options")) return Choice::OPTIONS;
		if (_tab.hovered("update"))  return Choice::UPDATE;
		if (_tab.hovered("status"))  return Choice::STATUS;
		return Choice::RUNTIME;
	}

	Software_tabs_dialog(Storage_target  const &storage_target,
	                     Presets         const &presets,
	                     Software_status const &status)
	:
		_storage_target(storage_target), _presets(presets), _status(status)
	{ }

	void generate(Xml_generator &xml) const
	{
		bool const presets_avail = _storage_target.valid() && _presets.available();
		bool const options_avail = _storage_target.valid();
		bool const update_avail  = _storage_target.valid();
		bool const status_avail  = _status.software_status_available();

		gen_named_node(xml, "frame", "software_tabs", [&] {

			xml.node("hbox", [&] {

				auto gen_tab = [&] (auto id, Choice choice, auto label, bool avail)
				{
					gen_named_node(xml, "button", id, [&] {

						if (_choice == choice)
							xml.attribute("selected", "yes");

						if (!avail)
							xml.attribute("style", "unimportant");

						xml.node("label", [&] {
							xml.attribute("text", label); });
					});
				};

				gen_tab("presets", Choice::PRESETS, "Presets", presets_avail);
				gen_tab("runtime", Choice::RUNTIME, "Runtime", true);
				gen_tab("options", Choice::OPTIONS, "Options", options_avail);
				gen_tab("update",  Choice::UPDATE,  "Update",  update_avail);
				gen_tab("status",  Choice::STATUS,  "Status",  status_avail);
			});
		});
	}

	Hover_result hover(Xml_node hover)
	{
		return _tab.match(hover, "frame", "hbox", "button", "name");
	}

	bool hovered() const { return _tab._hovered.valid(); }

	void click() { _choice = _hovered_choice(); }

	bool presets_selected() const { return _choice == Choice::PRESETS; }
	bool runtime_selected() const { return _choice == Choice::RUNTIME; }
	bool options_selected() const { return _choice == Choice::OPTIONS; }
	bool update_selected()  const { return _choice == Choice::UPDATE; }
	bool status_selected()  const { return _choice == Choice::STATUS; }
};

#endif /* _VIEW__SOFTWARE_TABS_DIALOG_H_ */
