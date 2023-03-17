/*
 * \brief  Dialog for browsing a depot index
 * \author Norman Feske
 * \date   2023-03-21
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__INDEX_MENU_DIALOG_H_
#define _VIEW__INDEX_MENU_DIALOG_H_

#include <view/dialog.h>
#include <model/index_menu.h>

namespace Sculpt { struct Index_menu_dialog; }


struct Sculpt::Index_menu_dialog
{
	public:

		using User         = Depot::Archive::User;
		using Index        = Attached_rom_dataspace;
		using Hover_result = Hoverable_item::Hover_result;
		using Name         = Start_name;
		using Entry_text   = String<100>;

		struct Policy : Interface
		{
			virtual void gen_index_menu_entry(Xml_generator            &,
			                                  Hoverable_item     const &,
			                                  Hoverable_item::Id const &,
			                                  Entry_text         const &,
			                                  Path               const &pkg_path,
			                                  char const *style = "radio") const = 0;
		};

		static void gen_title(Xml_generator        &xml,
		                      Hoverable_item const &item,
		                      Start_name     const &name,
		                      Start_name     const &text)
		{
			gen_named_node(xml, "hbox", name, [&] () {
				gen_named_node(xml, "float", "left", [&] {
					xml.attribute("west", "yes");
					xml.node("hbox", [&] {
						xml.node("float", [&] {
							gen_named_node(xml, "button", "back", [&] {
								xml.attribute("selected", "yes");
								xml.attribute("style", "back");
								item.gen_hovered_attr(xml, name);
								xml.node("hbox", [&] { });
							});
						});
						gen_named_node(xml, "label", "label", [&] {
							xml.attribute("font", "title/regular");
							xml.attribute("text", Path(text));
						});
					});
				});
				gen_named_node(xml, "hbox", "right", [&] { });
			});
		}

	private:

		Index const &_index;

		Index_menu _menu { };

		Hoverable_item _item { };

		bool _pkg_selected = false;

	public:

		Index_menu_dialog(Index const &index) : _index(index) { }

		void generate(Xml_generator &xml, User const &user, Policy const &policy) const
		{
			if (_menu.level()) {
				Name const title(" ", _menu, " ");
				gen_title(xml, _item, "back", title);
			}

			unsigned cnt = 0;
			_menu.for_each_item(_index.xml(), user, [&] (Xml_node const &item) {

				Hoverable_item::Id const id(cnt);

				if (item.has_type("index")) {
					auto const name = item.attribute_value("name", Name());
					policy.gen_index_menu_entry(xml, _item, id, Name(name, " ..."), "");
				}

				if (item.has_type("pkg")) {
					auto const path = item.attribute_value("path", Depot::Archive::Path());
					auto const name = Depot::Archive::name(path);

					policy.gen_index_menu_entry(xml, _item, id, name, path);
				}
				cnt++;
			});
		}

		template <typename ENTER_PKG_FN, typename LEAVE_PKG_FN>
		void click(User         const &user,
		           ENTER_PKG_FN const &enter_pkg_fn,
		           LEAVE_PKG_FN const &leave_pkg_fn)
		{
			Hoverable_item::Id const clicked = _item._hovered;

			/* go one menu up */
			if (clicked == "back") {
				_menu._selected[_menu._level] = Index_menu::Name();
				_menu._level--;
				_pkg_selected = false;
				leave_pkg_fn();
			} else {

				/* enter sub menu of index */
				if (_menu._level < Index_menu::MAX_LEVELS - 1) {

					unsigned cnt = 0;
					_menu.for_each_item(_index.xml(), user, [&] (Xml_node item) {

						if (clicked == Hoverable_item::Id(cnt)) {

							if (item.has_type("index")) {

								Index_menu::Name const name =
									item.attribute_value("name", Index_menu::Name());

								_menu._selected[_menu._level] = name;
								_menu._level++;

							} else if (item.has_type("pkg")) {

								_pkg_selected = true;
								enter_pkg_fn(item);
							}
						}
						cnt++;
					});
				}
			}
		}

		Hover_result hover(Xml_node const &hover)
		{
			return _item.match(hover, "hbox", "name");
		}

		bool hovered() const { return _item._hovered.valid();  }

		bool top_level() const { return (_menu.level() == 0) && !_pkg_selected; }

		bool pkg_selected() const { return _pkg_selected; }

		void deselect_pkg() { _pkg_selected = false; }

		void reset()
		{
			_item._hovered = Hoverable_item::Id();
			_menu = { };
			_pkg_selected = false;
		}
};

#endif /* _VIEW__INDEX_MENU_DIALOG_H_ */
