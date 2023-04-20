/*
 * \brief  Dialog for adding software components
 * \author Norman Feske
 * \date   2023-03-21
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__SOFTWARE_ADD_DIALOG_H_
#define _VIEW__SOFTWARE_ADD_DIALOG_H_

#include <model/build_info.h>
#include <model/nic_state.h>
#include <model/index_update_queue.h>
#include <model/index_menu.h>
#include <view/depot_users_dialog.h>
#include <view/index_menu_dialog.h>
#include <view/index_pkg_dialog.h>
#include <view/component_add_dialog.h>

namespace Sculpt { struct Software_add_dialog; }


struct Sculpt::Software_add_dialog : private Index_menu_dialog::Policy
{
	using Depot_users         = Depot_users_dialog::Depot_users;
	using User                = Depot_users_dialog::User;
	using Url                 = Depot_users_dialog::Url;
	using User_properties     = Depot_users_dialog::User_properties;
	using Index               = Index_menu_dialog::Index;
	using Construction_info   = Component::Construction_info;
	using Construction_action = Component::Construction_action;
	using Entry_text          = Index_menu_dialog::Entry_text;

	Build_info const _build_info;

	Sculpt_version     const  _sculpt_version;
	Nic_state          const &_nic_state;
	Index_update_queue const &_index_update_queue;
	Download_queue     const &_download_queue;
	Construction_info  const &_construction_info;

	struct Action : Interface
	{
		virtual void query_index        (User const &) = 0;
		virtual void update_sculpt_index(User const &, Verify) = 0;
	};

	Action &_action;

	Construction_action &_construction_action;

	Depot_users_dialog   _users;
	Index_menu_dialog    _menu;
	Index_pkg_dialog     _pkg { _nic_state };
	Component_add_dialog _component_add;

	Path _index_path() const { return Path(_users.selected(), "/index/", _sculpt_version); }

	bool _index_update_in_progress() const
	{
		using Update = Index_update_queue::Update;

		bool result = false;
		_index_update_queue.with_update(_index_path(), [&] (Update const &update) {
			if (update.active())
				result = true; });

		return result;
	}

	Hoverable_item _check { };

	using Hover_result = Hoverable_item::Hover_result;

	Software_add_dialog(Build_info           const &build_info,
	                    Sculpt_version       const &sculpt_version,
	                    Nic_state            const &nic_state,
	                    Index_update_queue   const &index_update_queue,
	                    Index                const &index,
	                    Download_queue       const &download_queue,
	                    Runtime_config       const &runtime_config,
	                    Construction_info    const &construction_info,
	                    Depot_users          const &depot_users,
	                    Depot_users_dialog::Action &depot_users_action,
	                    Action                     &action,
	                    Construction_action        &construction_action)
	:
		_build_info(build_info), _sculpt_version(sculpt_version),
		_nic_state(nic_state), _index_update_queue(index_update_queue),
		_download_queue(download_queue), _construction_info(construction_info),
		_action(action), _construction_action(construction_action),
		_users(depot_users, _build_info.depot_user, depot_users_action),
		_menu(index), _component_add(construction_info, runtime_config)
	{ }

	static void _gen_vspacer(Xml_generator &xml, char const *name)
	{
		gen_named_node(xml, "label", name, [&] () {
			xml.attribute("text", " ");
			xml.attribute("font", "annotation/regular");
		});
	}

	void _gen_horizontal_spacer(Xml_generator &xml) const
	{
		gen_named_node(xml, "label", "spacer", [&] {
			xml.attribute("min_ex", 35); });
	}

	/**
	 * Index_menu_dialog::Policy
	 */
	void gen_index_menu_entry(Xml_generator            &xml,
	                          Hoverable_item     const &item,
	                          Hoverable_item::Id const &id,
	                          Entry_text         const &text,
	                          Path               const &pkg_path,
	                          char const *style = "radio") const override
	{
		bool pkg_selected   = false;
		bool pkg_installing = false;

		Entry_text label { " ", text };

		if (pkg_path.length() > 1) {
			pkg_installing = _download_queue.in_progress(pkg_path);

			_construction_info.with_construction([&] (Component const &component) {
				if (component.path == pkg_path)
					pkg_selected = true;
			});

			label = { Pretty(label), " (", Depot::Archive::version(pkg_path), ")",
			          pkg_installing ? " installing... " : "... " };
		}

		gen_named_node(xml, "hbox", id, [&] () {

			gen_named_node(xml, "float", "left", [&] () {
				xml.attribute("west", "yes");

				xml.node("hbox", [&] {
					xml.node("float", [&] {
						gen_named_node(xml, "button", "button", [&] {

							if (pkg_selected)
								xml.attribute("selected", "yes");

							xml.attribute("style", style);
							item.gen_hovered_attr(xml, id);
							xml.node("hbox", [&] { });
						});
					});
					gen_named_node(xml, "label", "name", [&] {
						xml.attribute("text", label); });

					gen_item_vspace(xml, "vspace");
				});
			});

			gen_named_node(xml, "hbox", "right", [&] { });
		});

		if (pkg_selected && !pkg_installing) {
			_construction_info.with_construction([&] (Component const &component) {
				_pkg.generate(xml, Hoverable_item::Id(id, " details"), component,
				              _users.selected_user_properties());
			});
		}
	}

	bool _component_add_dialog_visible() const
	{
		bool result = false;
		if (_menu.pkg_selected())
			_construction_info.with_construction([&] (Component const &component) {
				if (component.blueprint_info.ready_to_deploy())
					result = true; });
		return result;
	}

	void _gen_add_dialog(Xml_generator &xml) const
	{
		gen_named_node(xml, "frame", "add_dialog", [&] {
			xml.node("vbox", [&] {
				_users.generate(xml);

				User_properties const properties = _users.selected_user_properties();

				bool const offer_index_update = _users.one_selected()
				                             && _menu.top_level()
				                             && _nic_state.ready()
				                             && properties.download_url;
				if (offer_index_update) {
					_gen_vspacer(xml, "above check");
					gen_named_node(xml, "float", "check", [&] {
						gen_named_node(xml, "button", "check", [&] {
							if (_index_update_in_progress()) {
								xml.attribute("selected", "yes");
								xml.attribute("style", "unimportant");
							}
							xml.node("label", [&] {
								auto const text = properties.public_key
								                ? "  Update Index  "
								                : "  Update unverfied Index  ";
								xml.attribute("text", text); });
						});
					});
					_gen_vspacer(xml, "below check");
				}
			});
		});

		if (_users.unfolded())
			return;

		_gen_vspacer(xml, "spacer4");
		_gen_vspacer(xml, "spacer5");
		gen_named_node(xml, "float", "index", [&] {
			xml.node("frame", [&] {
				xml.node("vbox", [&] {
					_gen_horizontal_spacer(xml);

					if (_component_add_dialog_visible())
						_component_add.generate(xml);
					else
						_menu.generate(xml, _users.selected(), *this);
				});
			});
		});
	}

	void _reset_menu()
	{
		_component_add.reset();
		_menu.reset();
	}

	void generate(Xml_generator &xml) const
	{
		gen_named_node(xml, "vbox", "add", [&] {
			_gen_add_dialog(xml); });
	}

	Hover_result hover(Xml_node const &hover)
	{
		return Dialog::any_hover_changed(
			match_sub_dialog(hover, _users, "vbox", "frame", "vbox"),
			match_sub_dialog(hover, _menu,  "vbox", "float", "frame", "vbox"),
			match_sub_dialog(hover, _pkg,  "vbox", "float", "frame", "vbox"),
			match_sub_dialog(hover, _component_add,  "vbox", "float", "frame", "vbox"),
			_check.match(hover, "vbox", "frame", "vbox", "float", "button", "name")
		);
	}

	bool hovered() const
	{
		return _users.hovered() || _menu.hovered() || _pkg.hovered() || _component_add.hovered();
	}

	bool keyboard_needed() const { return _users.keyboard_needed(); }

	void click()
	{
		if (_users.hovered()) {
			_users.click([&] (User const &selected_user) {
				_action.query_index(selected_user);
				_reset_menu();
			});
		}

		Verify const verify { _users.selected_user_properties().public_key };

		if (_component_add_dialog_visible()) {

			if (_component_add.hovered())
				_component_add.click(_construction_action,
					[&] /* leave */ {
						_construction_action.discard_construction();
						_component_add.reset();
						_menu.one_level_back();
					}
				);

		} else {

			if (_menu.hovered())
				_menu.click(_users.selected(),

					[&] /* enter pkg */ (Xml_node const &item) {

						auto path = item.attribute_value("path", Component::Path());
						auto info = item.attribute_value("info", Component::Info());

						_construction_action.new_construction(path, verify, info);
					},

					[&] /* leave pkg */ { _construction_action.discard_construction(); }
				);

			if (_pkg.hovered() && !_component_add_dialog_visible())
				_pkg.click([&] /* install */ {
					_construction_action.trigger_pkg_download(); });
		}

		if (_check.hovered("check") && !_index_update_in_progress())
			_action.update_sculpt_index(_users.selected(), verify);
	}

	void clack()
	{
		if (_component_add_dialog_visible() && _component_add.hovered())
			_component_add.clack([&] /* launch */ {
				_construction_action.launch_construction();
				_reset_menu(); });
	}

	void handle_key(Codepoint c) { _users.handle_key(c); }
};

#endif /* _VIEW__SOFTWARE_ADD_DIALOG_H_ */
