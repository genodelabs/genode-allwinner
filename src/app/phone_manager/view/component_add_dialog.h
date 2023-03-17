/*
 * \brief  Dialog for configuring a new component deployed from a depot package
 * \author Norman Feske
 * \date   2023-03-23
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__COMPONENT_ADD_DIALOG_H_
#define _VIEW__COMPONENT_ADD_DIALOG_H_

#include <view/index_menu_dialog.h>
#include <view/pd_route_dialog.h>
#include <view/resource_dialog.h>

namespace Sculpt { struct Component_add_dialog; }


struct Sculpt::Component_add_dialog
{
	using Hover_result        = Hoverable_item::Hover_result;
	using Name                = Start_name;
	using Construction_info   = Component::Construction_info;
	using Construction_action = Component::Construction_action;

	Construction_info const &_construction_info;
	Runtime_config    const &_runtime_config;

	Hoverable_item   _item        { };
	Hoverable_item   _route_item  { };
	Activatable_item _launch_item { };

	Constructible<Route::Id> _selected_route { };

	Pd_route_dialog _pd_route;

	Constructible<Resource_dialog> _resources { };

	static void _gen_info_label(Xml_generator &xml, char const *name,
	                            Component::Info const &info)
	{
		gen_named_node(xml, "label", name, [&] {
			xml.attribute("font", "annotation/regular");
			xml.attribute("text", Component::Info(" ", info, " ")); });
	}

	static void _gen_pkg_info(Xml_generator &xml, Component const &component)
	{
		using Info = Component::Info;
		if (component.info.length() > 1) {
			gen_named_node(xml, "label", "info", [&] {
				xml.attribute("text", Info(" ", component.info, " ")); });

			_gen_info_label(xml, "pad1", "");
		}
		Info const pkg(Depot::Archive::name(component.path), "/",
		               Depot::Archive::version(component.path));
		_gen_info_label(xml, "path", pkg);
	}

	bool _route_selected(Route::Id const &id) const
	{
		return _selected_route.constructed() && id == _selected_route->string();
	}

	bool _resource_dialog_selected() const
	{
		return _route_selected("resources");
	}

	template <typename FN>
	void _apply_to_selected_route(Construction_action &action, FN const &fn)
	{
		unsigned cnt = 0;
		action.apply_to_construction([&] (Component &component) {
			component.routes.for_each([&] (Route &route) {
				if (_route_selected(Route::Id(cnt++)))
					fn(route); }); });
	}

	void _gen_route_entry(Xml_generator &xml,
	                      Name const &name, Name const &text,
	                      bool selected, char const *style = "radio") const
	{
		gen_named_node(xml, "hbox", name, [&] {

			gen_named_node(xml, "float", "left", [&] {
				xml.attribute("west", "yes");

				xml.node("hbox", [&] {
					gen_named_node(xml, "float", "icon", [&] {
						xml.node("button", [&] {

							if (selected)
								xml.attribute("selected", "yes");

							xml.attribute("style", style);
							_route_item.gen_hovered_attr(xml, name);
							xml.node("hbox", [&] { });
						});
					});
					gen_named_node(xml, "label", "name", [&] {
						xml.attribute("text", Path(" ", text)); });
				});
			});

			gen_item_vspace(xml, "right");
		});
	}

	void _gen_pkg_elements(Xml_generator &xml, Component const &component) const
	{
		using Info = Component::Info;

		Index_menu_dialog::gen_title(xml, _item, "back", Name(" Add ", Pretty(component.name)));

		_gen_pkg_info(xml, component);

		_gen_info_label(xml, "resources", Info(Capacity{component.ram}, " ",
		                                       component.caps, " caps"));
		_gen_info_label(xml, "pad2", "");

		unsigned cnt = 0;
		component.routes.for_each([&] (Route const &route) {

			Route::Id const id(cnt++);

			bool const selected = _route_selected(id);
			bool const defined  = route.selected_service.constructed();

			if (!_selected_route.constructed() || selected) {
				gen_named_node(xml, "frame", id, [&] {

					xml.node("vbox", [&] {

						if (!selected)
							_gen_route_entry(xml, id,
							                 defined ? Info(route.selected_service->info)
							                         : Info(route),
							                 defined);

						/*
						 * List of routing options
						 */
						if (selected) {
							_gen_route_entry(xml, "back", Info(route), true, "back");

							unsigned cnt = 0;
							_runtime_config.for_each_service([&] (Service const &service) {

								Hoverable_item::Id const id("service.", cnt++);

								bool const service_selected =
									route.selected_service.constructed() &&
									id == route.selected_service_id;

								if (service.type == route.required)
									_gen_route_entry(xml, id, service.info, service_selected);
							});
						}
					});
				});
			}
		});

		_pd_route.generate(xml);

		if (_resources.constructed()) {

			xml.node("frame", [&] {
				xml.node("vbox", [&] {

					bool const selected = _route_selected("resources");

					if (!selected)
						_gen_route_entry(xml, "resources",
						                 "Resource assignment ...", false, "enter");

					if (selected) {
						_gen_route_entry(xml, "back", "Resource assignment ...",
						                 true, "back");

						_resources->generate(xml);
					}
				});
			});
		}

		/*
		 * Display "Add component" button once all routes are defined
		 */
		if (component.all_routes_defined()) {
			_gen_info_label(xml, "pad button", "");
			gen_named_node(xml, "button", "launch", [&] {
				_launch_item.gen_button_attr(xml, "launch");
				xml.node("label", [&] {
					xml.attribute("text", "Add component"); });
			});
		}
	}

	Component_add_dialog(Construction_info const &construction_info,
	                     Runtime_config    const &runtime_config)
	:
		_construction_info(construction_info), _runtime_config(runtime_config),
		_pd_route(runtime_config)
	{ }

	void generate(Xml_generator &xml) const
	{
		_construction_info.with_construction([&] (Component const &component) {
			_gen_pkg_elements(xml, component); });
	}

	Hover_result hover(Xml_node const &hover)
	{
		Dialog::Hover_result const hover_result = Dialog::any_hover_changed(
			_item       .match(hover, "hbox", "name"),
			_launch_item.match(hover, "button", "name"),
			_route_item .match(hover, "frame", "vbox", "hbox", "name"));

		_pd_route.hover(hover, "frame", "vbox", "hbox", "name");

		if (_resources.constructed() &&
		    hover_result == Dialog::Hover_result::UNMODIFIED)
			return _resources->match_sub_dialog(hover, "frame", "vbox", "frame", "vbox");

		return hover_result;
	}

	template <typename LEAVE_FN>
	void click(Construction_action &action, LEAVE_FN const &leave_fn)
	{
		if (_item.hovered("back")) {
			leave_fn();
			return;
		}

		if (_launch_item.hovered("launch")) {
			_launch_item.propose_activation_on_click();
			return;
		}

		Route::Id const clicked_route = _route_item._hovered;

		/* select route to present routing options */
		if (!_selected_route.constructed()) {
			if (clicked_route.valid()) {
				_selected_route.construct(clicked_route);
				return;
			}
		}

		if (!_selected_route.constructed())
			return;

		/*
		 * Route selected
		 */

		/* close selected route */
		if (clicked_route == "back") {
			_selected_route.destruct();
			_pd_route.reset();

		} else if (_resource_dialog_selected()) {

			bool const clicked_on_different_route = clicked_route.valid()
			                                     && (clicked_route != "");
			if (clicked_on_different_route) {

				/* close resource dialog */
				_selected_route.construct(clicked_route);

			} else {

				if (_resources.constructed())
					action.apply_to_construction([&] (Component &component) {
						_resources->click(component); });
			}

		} else {

			bool clicked_on_selected_route = false;

			_apply_to_selected_route(action, [&] (Route &route) {

				unsigned cnt = 0;
				_runtime_config.for_each_service([&] (Service const &service) {

					Hoverable_item::Id const id("service.", cnt++);

					if (clicked_route == id) {

						bool const clicked_service_already_selected =
							route.selected_service.constructed() &&
							id == route.selected_service_id;

						if (clicked_service_already_selected) {

							/* clear selection */
							route.selected_service.destruct();
							route.selected_service_id = Hoverable_item::Id();

						} else {

							/* select different service */
							route.selected_service.construct(service);
							route.selected_service_id = id;
						}

						_selected_route.destruct();

						clicked_on_selected_route = true;
					}
				});
			});

			/* select different route */
			if (!clicked_on_selected_route && clicked_route.valid())
				_selected_route.construct(clicked_route);

			action.apply_to_construction([&] (Component &component) {
				_pd_route.click(component); });
		}
	}

	template <typename LAUNCH_FN>
	void clack(LAUNCH_FN const &launch_fn)
	{
		_launch_item.confirm_activation_on_clack();

		if (_launch_item.activated("launch"))
			launch_fn();
	}

	void reset()
	{
		_item._hovered = Hoverable_item::Id();
		_route_item._hovered = Hoverable_item::Id();
		_launch_item.reset();
		_selected_route.destruct();
		_resources.destruct();
		_pd_route.reset();
	}

	bool hovered() const { return _item._hovered.valid()
	                           || _route_item._hovered.valid()
	                           || _launch_item._hovered.valid(); }
};

#endif /* _VIEW__COMPONENT_ADD_DIALOG_H_ */
