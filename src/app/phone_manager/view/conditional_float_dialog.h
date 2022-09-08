/*
 * \brief  Conditionally visible dialog
 * \author Norman Feske
 * \date   2022-05-20
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__CONDITIONAL_FLOAT_DIALOG_H_
#define _VIEW__CONDITIONAL_FLOAT_DIALOG_H_

#include <view/dialog.h>


namespace Sculpt { template <typename> struct Conditional_float_dialog; }


/*
 * Float widget that generates its content only if a given condition is true.
 */
template <typename DIALOG>
struct Sculpt::Conditional_float_dialog : Noncopyable
{
	using Hover_result = Hoverable_item::Hover_result;
	using Float_id     = String<16>;

	Float_id const float_id;

	Hoverable_item _float { };

	DIALOG dialog;

	Hover_result hover(Xml_node hover)
	{
		Hover_result const float_hover = _float.match(hover, "float", "name");

		Hover_result dialog_hover = Hover_result::UNMODIFIED;
		hover.with_optional_sub_node("float", [&] (Xml_node const &hover) {
			dialog_hover = dialog.hover(hover); });

		return Dialog::any_hover_changed(float_hover, dialog_hover);
	}

	void generate_conditional(Xml_generator &xml, bool condition) const
	{
		gen_named_node(xml, "float", float_id, [&] {
			xml.attribute("east", "yes");
			xml.attribute("west", "yes");
			if (condition)
				dialog.generate(xml); });
	}

	bool hovered() const
	{
		return _float.hovered(float_id);
	}

	void click() { dialog.click(); }
	void clack() { dialog.clack(); }

	template <typename... ARGS>
	Conditional_float_dialog(Float_id id, ARGS &&... args)
	: float_id(id), dialog(args...) { }
};

#endif /* _VIEW__CONDITIONAL_FLOAT_DIALOG_H_ */
