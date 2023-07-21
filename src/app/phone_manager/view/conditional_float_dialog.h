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
struct Sculpt::Conditional_float_dialog : Widget<Float>
{
	Id const id;

	using Hover_result = Hoverable_item::Hover_result;
	using Float_id     = String<16>;

	Hoverable_item _float { };

	Hosted<Float, DIALOG> dialog;

	Hover_result hover(Xml_node hover)
	{
		Hover_result const float_hover = _float.match(hover, "float", "name");

		Hover_result dialog_hover = Hover_result::UNMODIFIED;
		hover.with_optional_sub_node("float", [&] (Xml_node const &hover) {
			dialog_hover = dialog.hover(hover); });

		return Deprecated_dialog::any_hover_changed(float_hover, dialog_hover);
	}

	void generate_conditional(Xml_generator &xml, bool condition) const
	{
		gen_named_node(xml, "float", id.value, [&] {
			xml.attribute("east", "yes");
			xml.attribute("west", "yes");
			if (condition)
				dialog.generate(xml); });
	}

	template <typename... ARGS>
	void view(Scope<Float> &s, bool condition, ARGS &&... args) const
	{
		s.attribute("east", "yes");
		s.attribute("west", "yes");
		if (condition)
			s.widget(dialog, args...);
	}

	bool hovered() const
	{
		return _float.hovered(id.value);
	}

	template <typename... ARGS>
	void click(Clicked_at const &at, ARGS &&... args) { dialog.propagate(at, args...); }

	template <typename... ARGS>
	void clack(Clacked_at const &at, ARGS &&... args) { dialog.propagate(at, args...); }

	template <typename... ARGS>
	void drag (Dragged_at const &at, ARGS &&... args) { dialog.propagate(at, args...); }

	void click() { dialog.click(); }
	void clack() { dialog.clack(); }

	template <typename... ARGS>
	Conditional_float_dialog(Id const &id, ARGS &&... args)
	: id(id), dialog(id, args...) { }
};


namespace Sculpt {

	template <typename DIALOG>
	struct Conditional_dialog : Hosted<Vbox, Conditional_float_dialog<DIALOG> >
	{
		template <typename... ARGS>
		Conditional_dialog(Id const &id, ARGS &&... args)
		:
			/*
			 * The first 'id' argument is held at 'Hosted', the second 'id'
			 * argument is held by 'Conditional_float_dialog'. The 'args' are
			 * forwarded to 'DIALOG'.
			 */
			Hosted<Vbox, Conditional_float_dialog<DIALOG> >(id, id, args...)
		{ }
	};
}

#endif /* _VIEW__CONDITIONAL_FLOAT_DIALOG_H_ */
