/*
 * \brief  Dialog for software update
 * \author Norman Feske
 * \date   2023-01-23
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__SOFTWARE_UPDATE_DIALOG_H_
#define _VIEW__SOFTWARE_UPDATE_DIALOG_H_

#include <model/build_info.h>
#include <model/download_queue.h>
#include <model/index_update_queue.h>
#include <view/dialog.h>

namespace Sculpt { struct Software_update_dialog; }


struct Sculpt::Software_update_dialog
{
	using Depot_users = Attached_rom_dataspace;
	using Image_index = Attached_rom_dataspace;
	using User        = Depot::Archive::User;
	using Url         = String<128>;
	using Version     = String<16>;

	Build_info const _build_info;

	Download_queue       const &_download_queue;
	Index_update_queue   const &_index_update_queue;
	File_operation_queue const &_file_operation_queue;
	Depot_users          const &_depot_users;
	Image_index          const &_image_index;

	User _selected_user = _build_info.depot_user;

	Path _last_installed { };
	Path _last_selected  { };

	bool _users_unfolded = false;

	Path _index_path() const { return Path(_selected_user, "/image/index"); }

	bool _index_update_in_progress() const
	{
		using Update = Index_update_queue::Update;

		bool result = false;
		_index_update_queue.with_update(_index_path(), [&] (Update const &update) {
			if (update.active())
				result = true; });

		return result;
	}

	Path _image_path(Version const &version) const
	{
		return Path(_selected_user, "/image/sculpt-", _build_info.board, "-", version);
	}

	bool _installing() const
	{
		return _file_operation_queue.copying_to_path("/rw/boot");
	};

	Hoverable_item _user      { };
	Hoverable_item _check     { };
	Hoverable_item _version   { };
	Hoverable_item _operation { };

	struct Action : Interface
	{
		virtual void query_image_index(User const &user) = 0;
		virtual void trigger_image_download(Path const &) = 0;
		virtual void update_image_index(User const &) = 0;
		virtual void install_boot_image(Path const &) = 0;
	};

	Action &_action;

	using Hover_result = Hoverable_item::Hover_result;

	Software_update_dialog(Build_info           const &build_info,
	                       Download_queue       const &download_queue,
	                       Index_update_queue   const &index_update_queue,
	                       File_operation_queue const &file_operation_queue,
	                       Depot_users          const &depot_users,
	                       Image_index          const &image_index,
	                       Action &action)
	:
		_build_info(build_info),
		_download_queue(download_queue),
		_index_update_queue(index_update_queue),
		_file_operation_queue(file_operation_queue),
		_depot_users(depot_users),
		_image_index(image_index),
		_action(action)
	{ }

	Url _user_url(Xml_node const &user) const
	{
		if (!user.has_sub_node("url"))
			return { };

		Url const url = user.sub_node("url").decoded_content<Url>();

		/*
		 * Ensure that the URL does not contain any '"' character because it will
		 * be taken as an XML attribute value.
		 */
		for (char const *s = url.string(); *s; s++)
			if (*s == '"')
				return { };

		User const name = user.attribute_value("name", User());

		return Url(url, "/", name);
	}

	void _gen_vspacer(Xml_generator &xml, char const *name) const
	{
		gen_named_node(xml, "label", name, [&] () {
			xml.attribute("text", " ");
			xml.attribute("font", "annotation/regular");
		});
	}

	void _gen_user_entry(Xml_generator &xml, Xml_node const user, bool last) const
	{
		User const name     = user.attribute_value("name", User());
		bool const selected = (name == _selected_user);

		if (!selected && !_users_unfolded)
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
						xml.attribute("text", Path(" ", _user_url(user))); });
				});
			});
			gen_named_node(xml, "hbox", "right", [&] () { });
		});
		if (_users_unfolded && !last)
			_gen_vspacer(xml, String<64>("below ", name).string());
	}

	void _gen_user_selection(Xml_generator &xml) const
	{
		Xml_node const depot_users = _depot_users.xml();

		size_t remain_count = depot_users.num_sub_nodes();

		gen_named_node(xml, "frame", "user_selection", [&] () {
			xml.node("vbox", [&] () {
				depot_users.for_each_sub_node("user", [&] (Xml_node user) {
					bool const last = (--remain_count == 0);
					_gen_user_entry(xml, user, last); }); }); });
	}

	void _gen_image_main(Xml_generator &xml, Xml_node const &image) const
	{
		Version const version = image.attribute_value("version", Version());
		bool    const present = image.attribute_value("present", false);
		Path    const path    = _image_path(version);

		struct Download_state
		{
			bool     in_progress;
			bool     failed;
			unsigned percent;
		};

		using Download = Download_queue::Download;

		auto state_from_download_queue = [&]
		{
			Download_state result { };
			_download_queue.with_download(path, [&] (Download const &download) {

				if (download.state == Download::State::DOWNLOADING)
					result.in_progress = true;

				if (download.state == Download::State::FAILED)
					result.failed = true;

				result.percent = download.percent;
			});
			return result;
		};

		Download_state const download_state = state_from_download_queue();

		gen_named_node(xml, "float", "label", [&] {
			xml.attribute("west", "yes");
			gen_named_node(xml, "label", "label", [&] {
				xml.attribute("text", String<50>("  ", version));
				xml.attribute("min_ex", "15");
			});
		});

		auto gen_status = [&] (auto message)
		{
			gen_named_node(xml, "float", "status", [&] {
				xml.node("label", [&] {
					xml.attribute("font", "annotation/regular");
					xml.attribute("text", message); }); });
		};

		if (image.has_sub_node("info")) {
			if (_last_selected == path)
				gen_status("Changes");
			else
				gen_status("...");
		}

		if (download_state.in_progress && download_state.percent)
			gen_status(String<16>(download_state.percent, "%"));

		if (download_state.failed)
			gen_status("unavailable");

		if (_last_installed == path) {
			if (_installing())
				gen_status("installing...");
			else
				gen_status("reboot to activate");
		}

		gen_named_node(xml, "float", "buttons", [&] {
			xml.attribute("east", "yes");
			xml.node("hbox", [&] {

				auto gen_button = [&] (auto id, bool selected, auto text)
				{
					gen_named_node(xml, "button", id, [&] {

						if (selected) {
							xml.attribute("selected", "yes");
							xml.attribute("style", "unimportant");
						}

						xml.node("label", [&] {
							xml.attribute("text", text); });
					});
				};

				if (present)
					gen_button("install", _installing(), "  Install  ");

				if (!present)
					gen_button("download", download_state.in_progress, "  Download  ");
			});
		});
	}

	void _gen_image_info(Xml_generator &xml, Xml_node const &image) const
	{
		gen_named_node(xml, "vbox", "main", [&] {

			unsigned line = 0;

			image.for_each_sub_node("info", [&] (Xml_node const &info) {

			 	/* limit changelog to a sensible maximum of lines  */
				if (++line > 8)
					return;

				using Text = String<80>;
				Text const text = info.attribute_value("text", Text());

				gen_named_node(xml, "float", String<16>(line), [&] {
					xml.attribute("west", "yes");
					xml.node("label", [&] {
						xml.attribute("text", text);
						xml.attribute("font", "annotation/regular");
					});
				});
			});
		});
	}

	void _gen_image_entry(Xml_generator &xml, Xml_node const &image) const
	{
		Version const version = image.attribute_value("version", Version());
		Path    const path    = _image_path(version);

		gen_named_node(xml, "frame", version, [&] {
			xml.attribute("style", "important");

			xml.node("vbox", [&] {

				gen_named_node(xml, "float", "main", [&] {
					xml.attribute("east", "yes");
					xml.attribute("west", "yes");
					_gen_image_main(xml, image);
				});

				if (path == _last_selected) {
					_gen_vspacer(xml, "above");
					gen_named_node(xml, "float", "info", [&] {
						_gen_image_info(xml, image); });
					_gen_vspacer(xml, "below");
				}
			});
		});
	}

	void _gen_image_list(Xml_generator &xml) const
	{
		Xml_node const index = _image_index.xml();

		index.for_each_sub_node("user", [&] (Xml_node const &user) {
			if (user.attribute_value("name", User()) == _selected_user)
				user.for_each_sub_node("image", [&] (Xml_node const &image) {
					_gen_image_entry(xml, image); }); });
	}

	void _gen_update_dialog(Xml_generator &xml) const
	{
		gen_named_node(xml, "frame", "update_dialog", [&] {
			xml.node("vbox", [&] {
				_gen_user_selection(xml);

				if (_users_unfolded)
					return;

				_gen_vspacer(xml, "spacer1");

				gen_named_node(xml, "float", "check", [&] {
					gen_named_node(xml, "button", "check", [&] {
						if (_index_update_in_progress()) {
							xml.attribute("selected", "yes");
							xml.attribute("style", "unimportant");
						}
						xml.node("label", [&] {
							xml.attribute("text", "  Check for Updates  "); });
					});
				});

				_gen_vspacer(xml, "spacer2");
			});
		});

		_gen_image_list(xml);
	}

	void generate(Xml_generator &xml) const
	{
		gen_named_node(xml, "vbox", "update", [&] {
			_gen_update_dialog(xml); });
	}

	Hover_result hover(Xml_node const &hover)
	{
		return Dialog::any_hover_changed(
			_user     .match(hover, "vbox", "frame", "vbox", "frame", "vbox", "hbox", "name"),
			_check    .match(hover, "vbox", "frame", "vbox", "float", "button", "name"),
			_version  .match(hover, "vbox", "frame", "name"),
			_operation.match(hover, "vbox", "frame", "vbox", "float", "float", "hbox", "button", "name")
		);
	}

	bool hovered() const { return _user._hovered.valid();  }

	void click()
	{
		if (_user._hovered.length() > 1) {

			if (_users_unfolded) {
				_selected_user = _user._hovered;
				_action.query_image_index(_selected_user);
				_users_unfolded = false;
			} else {
				_users_unfolded = true;
			}
		}

		if (_check.hovered("check") && !_index_update_in_progress())
			_action.update_image_index(_selected_user);

		if (_operation.hovered("download"))
			_action.trigger_image_download(_image_path(_version._hovered));

		if (_version._hovered.length() > 1)
			_last_selected = _image_path(_version._hovered);

		if (_operation.hovered("install") && !_installing()) {
			_last_installed = _image_path(_version._hovered);
			_action.install_boot_image(_last_installed);
		}
	}

	void clack() { }
};

#endif /* _VIEW__SOFTWARE_UPDATE_DIALOG_H_ */
