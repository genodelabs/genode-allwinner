/*
 * \brief  Dialog for presenting pkg details and the option for installation
 * \author Norman Feske
 * \date   2023-03-22
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__INDEX_PKG_DIALOG_H_
#define _VIEW__INDEX_PKG_DIALOG_H_

#include <model/nic_state.h>
#include <view/depot_users_dialog.h>

namespace Sculpt { struct Index_pkg_dialog; }


struct Sculpt::Index_pkg_dialog
{
	using Hover_result    = Hoverable_item::Hover_result;
	using Id              = Hoverable_item::Id;
	using User_properties = Depot_users_dialog::User_properties;

	Nic_state const &_nic_state;

	Index_pkg_dialog(Nic_state const &nic_state) : _nic_state(nic_state) { }

	Activatable_item _install_item { };

	static void _gen_info_label(Xml_generator &xml, char const *name,
	                            Component::Info const &info)
	{
		gen_named_node(xml, "label", name, [&] {
			xml.attribute("font", "annotation/regular");
			xml.attribute("text", Component::Info(" ", info, " ")); });
	}

	static void _gen_pkg_info(Xml_generator &xml, Component const &component)
	{
		if (component.info.length() > 1) {
			gen_named_node(xml, "label", "info", [&] {
				xml.attribute("text", Component::Info(" ", component.info, " ")); });

			_gen_info_label(xml, "pad1", "");
		}
		_gen_info_label(xml, "path", component.path);
	}

	void generate(Xml_generator &xml, Id const &id, Component const &component,
	              User_properties const &properties) const
	{
		if (!component.blueprint_info.known || component.blueprint_info.ready_to_deploy())
			return;

		gen_named_node(xml, "float", id, [&] {

			gen_named_node(xml, "vbox", "vbox", [&] {

				/*
				 * Package is installed but content is missing
				 *
				 * This can happen when the pkg's runtime is inconsistent with
				 * the content contained in the pkg's archives.
				 */
				if (component.blueprint_info.incomplete()) {
					_gen_info_label(xml, "pad2", "");
					_gen_info_label(xml, "path", component.path);
					_gen_info_label(xml, "pad3", "");
					xml.node("label", [&] {
						xml.attribute("text", "installed but incomplete"); });

					if (_nic_state.ready()) {
						_gen_info_label(xml, "pad4", "");

						auto const text = properties.public_key
						                ? " Reattempt Install "
						                : " Reattempt Install without Verification ";

						gen_named_node(xml, "float", "install", [&] () {
							xml.node("button", [&] () {
								_install_item.gen_button_attr(xml, "install");
								xml.node("label", [&] () {
									xml.attribute("text", text);
								});
							});
						});
					}
				}

				/*
				 * Package is missing but can be installed
				 */
				else if (component.blueprint_info.uninstalled() && _nic_state.ready()) {

					_gen_pkg_info(xml, component);
					_gen_info_label(xml, "pad2", "");

					auto const text = properties.public_key
					                ? " Install "
					                : " Install without Verification ";

					gen_named_node(xml, "float", "install", [&] {
						xml.node("button", [&] {
							_install_item.gen_button_attr(xml, "install");
							xml.node("label", [&] {
								xml.attribute("text", text);
							});
						});
					});
				}

				/*
				 * Package is missing and we cannot do anything about it
				 */
				else if (component.blueprint_info.uninstalled()) {
					_gen_info_label(xml, "pad2", "");
					_gen_info_label(xml, "path", component.path);
					_gen_info_label(xml, "pad3", "");
					xml.node("label", [&] {
						xml.attribute("text", "not installed"); });
				}

				_gen_info_label(xml, "pad4", "");
			});
		});
	}

	template <typename INSTALL_FN>
	void click(INSTALL_FN const install_fn)
	{
		if (_install_item.hovered("install"))
			install_fn();
	}

	Hover_result hover(Xml_node const &hover)
	{
		return _install_item.match(hover, "float", "vbox", "float", "button", "name");
	}

	bool hovered() const { return _install_item._hovered.valid(); }
};

#endif /* _VIEW__INDEX_PKG_DIALOG_H_ */
