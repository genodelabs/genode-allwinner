/*
 * \brief  Section dialog
 * \author Norman Feske
 * \date   2022-05-20
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__SECTION_DIALOG_H_
#define _VIEW__SECTION_DIALOG_H_

#include <view/dialog.h>


namespace Sculpt { struct Section_dialog; }


struct Sculpt::Section_dialog : Noncopyable, Dialog
{
	using Frame_id = String<16>;

	Frame_id const frame_id;

	Hoverable_item _frame { };

	enum class Detail { DEFAULT, SELECTED, MINIMIZED };

	Detail detail = Detail::DEFAULT;

	void reset() override { }

	Hover_result hover(Xml_node hover) override
	{
		return _frame.match(hover, "vbox", "name");
	}

	void _gen_label_attr(Xml_generator &xml) const
	{
		switch (detail) {
		case Detail::DEFAULT:
		case Detail::SELECTED:
			xml.attribute("font", "title/regular");
			break;
		case Detail::MINIMIZED:
			xml.attribute("font", "annotation/regular");
			break;
		}
	}

	void _gen_status_attr(Xml_generator &xml) const
	{
		switch (detail) {
		case Detail::DEFAULT:
		case Detail::SELECTED:
			break;
		case Detail::MINIMIZED:
			xml.attribute("font", "annotation/regular");
			break;
		}
	}

	template <typename FN>
	void _gen_frame(Xml_generator &xml, FN const &fn) const
	{
		gen_named_node(xml, "vbox", frame_id.string(), [&] {

			xml.node("float", [&] {

				xml.attribute("east", "yes");
				xml.attribute("west", "yes");

				xml.node("button", [&] {

					if (detail == Detail::MINIMIZED)
						xml.attribute("style", "unimportant");

					if (detail == Detail::SELECTED)
						xml.attribute("selected", "yes");

					fn();
				});
			});
		});
	}

	bool hovered() const
	{
		return _frame.hovered(frame_id);
	}

	bool selected() const { return detail == Detail::SELECTED; }

	Section_dialog(Frame_id id) : frame_id(id) { }
};

#endif /* _VIEW__SECTION_DIALOG_H_ */
