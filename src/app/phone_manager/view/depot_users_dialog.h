/*
 * \brief  Dialog for selecting a depot user
 * \author Norman Feske
 * \date   2023-03-17
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__DEPOT_USERS_DIALOG_H_
#define _VIEW__DEPOT_USERS_DIALOG_H_

#include <view/dialog.h>

namespace Sculpt { struct Depot_users_dialog; }


struct Sculpt::Depot_users_dialog
{
	public:

		using Depot_users  = Attached_rom_dataspace;
		using User         = Depot::Archive::User;
		using Url          = String<128>;
		using Hover_result = Hoverable_item::Hover_result;

	private:

		Depot_users const &_depot_users;

		User _selected;

		bool _unfolded = false;

		Hoverable_item _user { };

		Url _url(Xml_node const &user) const
		{
			if (!user.has_sub_node("url"))
				return { };

			Url const url = user.sub_node("url").decoded_content<Url>();

			/*
			 * Ensure that the URL does not contain any '"' character because
			 * it will be taken as an XML attribute value.
			 */
			for (char const *s = url.string(); *s; s++)
				if (*s == '"')
					return { };

			User const name = user.attribute_value("name", User());

			return Url(url, "/", name);
		}

		static void _gen_vspacer(Xml_generator &xml, char const *name)
		{
			gen_named_node(xml, "label", name, [&] () {
				xml.attribute("text", " ");
				xml.attribute("font", "annotation/regular");
			});
		}

		void _gen_entry(Xml_generator &xml, Xml_node const user, bool last) const
		{
			User const name     = user.attribute_value("name", User());
			bool const selected = (name == _selected);

			if (!selected && !_unfolded)
				return;

			gen_named_node(xml, "hbox", name, [&] () {
				gen_named_node(xml, "float", "left", [&] () {
					xml.attribute("west", "yes");
					xml.node("hbox", [&] () {
						gen_named_node(xml, "float", "button", [&] () {
							gen_named_node(xml, "button", "button", [&] () {

								if (selected)
									xml.attribute("selected", "yes");

								xml.attribute("style", "radio");
								xml.node("hbox", [&] () { });
							});
						});
						gen_named_node(xml, "label", "name", [&] () {
							xml.attribute("text", Path(" ", _url(user))); });
					});
				});
				gen_named_node(xml, "hbox", "right", [&] () { });
			});
			if (_unfolded && !last)
				_gen_vspacer(xml, String<64>("below ", name).string());
		}

		void _gen_selection(Xml_generator &xml) const
		{
			Xml_node const depot_users = _depot_users.xml();

			size_t remain_count = depot_users.num_sub_nodes();

			gen_named_node(xml, "frame", "user_selection", [&] () {
				xml.node("vbox", [&] () {
					depot_users.for_each_sub_node("user", [&] (Xml_node user) {
						bool const last = (--remain_count == 0);
						_gen_entry(xml, user, last); }); }); });
		}

	public:

		Depot_users_dialog(Depot_users const &depot_users,
		                   User        const &default_user)
		:
			_depot_users(depot_users), _selected(default_user)
		{ }

		User selected() const { return _selected; }

		void generate(Xml_generator &xml) const { _gen_selection(xml); }

		bool unfolded() const { return _unfolded; }

		template <typename FN>
		void click(FN const &select_fn)
		{
			if (_user._hovered.length() > 1) {

				if (_unfolded) {
					_selected = _user._hovered;
					select_fn(_selected);
					_unfolded = false;
				} else {
					_unfolded = true;
				}
			}
		}

		Hover_result hover(Xml_node const &hover)
		{
			return Dialog::any_hover_changed(
				_user.match(hover, "frame", "vbox", "hbox", "name")
			);
		}

		bool hovered() const { return _user._hovered.valid();  }
};

#endif /* _VIEW__DEPOT_USERS_DIALOG_H_ */
