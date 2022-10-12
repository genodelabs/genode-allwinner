/*
 * \brief  Sculpt system manager for a phone
 * \author Norman Feske
 * \date   2022-05-20
 *
 * Based on repos/gems/src/app/sculpt_manager/main.cc
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/component.h>
#include <base/heap.h>
#include <base/attached_rom_dataspace.h>
#include <os/reporter.h>
#include <gui_session/connection.h>
#include <vm_session/vm_session.h>
#include <timer_session/connection.h>
#include <io_port_session/io_port_session.h>
#include <event_session/event_session.h>
#include <capture_session/capture_session.h>
#include <gpu_session/gpu_session.h>
#include <pin_state_session/pin_state_session.h>
#include <pin_control_session/pin_control_session.h>

/* local includes */
#include <model/runtime_state.h>
#include <model/child_exit_state.h>
#include <model/sculpt_version.h>
#include <model/file_operation_queue.h>
#include <menu_view.h>
#include <managed_config.h>
#include <gui.h>
#include <storage.h>
#include <network.h>
#include <deploy.h>
#include <graph.h>
#include <view/device_section_dialog.h>
#include <view/phone_section_dialog.h>
#include <view/modem_power_dialog.h>
#include <view/pin_dialog.h>
#include <view/dialpad_dialog.h>
#include <view/current_call_dialog.h>
#include <view/outbound_dialog.h>
#include <view/storage_section_dialog.h>
#include <view/network_section_dialog.h>
#include <view/software_section_dialog.h>

namespace Sculpt { struct Main; }


struct Sculpt::Main : Input_event_handler,
                      Dialog::Generator,
                      Runtime_config_generator,
                      Storage::Target_user,
                      Network::Action,
                      Graph::Action,
                      Dialog,
                      Depot_query,
                      Menu_view::Hover_update_handler,
                      Modem_power_dialog::Action,
                      Pin_dialog::Action,
                      Dialpad_dialog::Action,
                      Current_call_dialog::Action,
                      Outbound_dialog::Action
{
	Env &_env;

	Heap _heap { _env.ram(), _env.rm() };

	Sculpt_version const _sculpt_version { _env };

	Registry<Child_state> _child_states { };

	Input::Seq_number _global_input_seq_number { };

	Gui::Connection _gui { _env, "input" };

	bool _gui_mode_ready = false;  /* becomes true once the graphics driver is up */

	Gui::Root _gui_root { _env, _heap, *this, _global_input_seq_number };

	Signal_handler<Main> _input_handler {
		_env.ep(), *this, &Main::_handle_input };

	void _handle_input()
	{
		_gui.input()->for_each_event([&] (Input::Event const &ev) {
			handle_input_event(ev); });
	}

	Signal_handler<Main> _gui_mode_handler {
		_env.ep(), *this, &Main::_handle_gui_mode };

	void _handle_gui_mode();

	bool _verbose_modem = false;

	Attached_rom_dataspace _config { _env, "config" };

	Signal_handler<Main> _config_handler {
		_env.ep(), *this, &Main::_handle_config };

	void _handle_config()
	{
		_config.update();

		Xml_node const config = _config.xml();

		_verbose_modem = config.attribute_value("verbose_modem", false);
	}

	Attached_rom_dataspace _leitzentrale_rom { _env, "leitzentrale" };

	Signal_handler<Main> _leitzentrale_handler {
		_env.ep(), *this, &Main::_handle_leitzentrale };

	void _handle_leitzentrale()
	{
		_leitzentrale_rom.update();

		_leitzentrale_visible = _leitzentrale_rom.xml().attribute_value("enabled", false);

		_handle_window_layout();
	}


	/***************************
	 ** Configuration loading **
	 ***************************/

	Prepare_version _prepare_version   { 0 };
	Prepare_version _prepare_completed { 0 };

	bool _prepare_in_progress() const
	{
		return _prepare_version.value != _prepare_completed.value;
	}


	Storage _storage { _env, _heap, _child_states, *this, *this, *this };

	/**
	 * Storage::Target_user interface
	 */
	void use_storage_target(Storage_target const &target) override
	{
		_storage._sculpt_partition = target;

		/* trigger loading of the configuration from the sculpt partition */
		_prepare_version.value++;

		_deploy.restart();

		generate_runtime_config();
	}

	Pci_info _pci_info { .modem_present = true };

	Network _network { _env, _heap, *this, _child_states, *this, _runtime_state, _pci_info };

	/**
	 * Network::Action interface
	 */
	void update_network_dialog() override
	{
		generate_dialog();
	}


	/************
	 ** Update **
	 ************/

	Attached_rom_dataspace _update_state_rom {
		_env, "report -> runtime/update/state" };

	void _handle_update_state();

	Signal_handler<Main> _update_state_handler {
		_env.ep(), *this, &Main::_handle_update_state };

	/**
	 * Condition for spawning the update subsystem
	 */
	bool _update_running() const
	{
		return _storage._sculpt_partition.valid()
		    && !_prepare_in_progress()
		    && _network.ready()
		    && _deploy.update_needed();
	}

	Download_queue _download_queue { _heap };

	File_operation_queue _file_operation_queue { _heap };

	Fs_tool_version _fs_tool_version { 0 };


	/*****************
	 ** Depot query **
	 *****************/

	Depot_query::Version _query_version { 0 };

	Expanding_reporter _depot_query_reporter { _env, "query", "depot_query"};

	/**
	 * Depot_query interface
	 */
	Depot_query::Version depot_query_version() const override
	{
		return _query_version;
	}

	Timer::Connection _timer { _env };

	Timer::One_shot_timeout<Main> _deferred_depot_query_handler {
		_timer, *this, &Main::_handle_deferred_depot_query };

	void _handle_deferred_depot_query(Duration)
	{
		if (_deploy._arch.valid()) {
			_query_version.value++;
			_depot_query_reporter.generate([&] (Xml_generator &xml) {
				xml.attribute("arch",    _deploy._arch);
				xml.attribute("version", _query_version.value);

				/* update query for blueprints of all unconfigured start nodes */
				_deploy.gen_depot_query(xml);
			});
		}
	}

	/**
	 * Depot_query interface
	 */
	void trigger_depot_query() override
	{
		/*
		 * Defer the submission of the query for a few milliseconds because
		 * 'trigger_depot_query' may be consecutively called several times
		 * while evaluating different conditions. Without deferring, the depot
		 * query component would produce intermediate results that take time
		 * but are ultimately discarded.
		 */
		_deferred_depot_query_handler.schedule(Microseconds{5000});
	}


	/*********************
	 ** Blueprint query **
	 *********************/

	Attached_rom_dataspace _blueprint_rom { _env, "report -> runtime/depot_query/blueprint" };

	Signal_handler<Main> _blueprint_handler {
		_env.ep(), *this, &Main::_handle_blueprint };

	void _handle_blueprint()
	{
		_blueprint_rom.update();

		Xml_node const blueprint = _blueprint_rom.xml();

		/*
		 * Drop intermediate results that will be superseded by a newer query.
		 * This is important because an outdated blueprint would be disregarded
		 * by 'handle_deploy' anyway while at the same time a new query is
		 * issued. This can result a feedback loop where blueprints are
		 * requested but never applied.
		 */
		if (blueprint.attribute_value("version", 0U) != _query_version.value)
			return;

		_deploy.handle_deploy();
	}


	/************
	 ** Deploy **
	 ************/

	Deploy::Prio_levels const _prio_levels { 4 };

	Attached_rom_dataspace _launcher_listing_rom {
		_env, "report -> /runtime/launcher_query/listing" };

	Launchers _launchers { _heap };

	Signal_handler<Main> _launcher_listing_handler {
		_env.ep(), *this, &Main::_handle_launcher_listing };

	void _handle_launcher_listing()
	{
		_launcher_listing_rom.update();

		Xml_node listing = _launcher_listing_rom.xml();
		if (listing.has_sub_node("dir")) {
			Xml_node dir = listing.sub_node("dir");

			/* let 'update_from_xml' iterate over <file> nodes */
			_launchers.update_from_xml(dir);
		}

		generate_dialog();
		_deploy._handle_managed_deploy();
	}

	Deploy _deploy { _env, _heap, _child_states, _runtime_state, *this, *this, *this,
	                 _launcher_listing_rom, _blueprint_rom, _download_queue };

	Attached_rom_dataspace _manual_deploy_rom { _env, "config -> deploy" };

	void _handle_manual_deploy()
	{
		_runtime_state.reset_abandoned_and_launched_children();
		_manual_deploy_rom.update();
		_deploy.update_managed_deploy_config(_manual_deploy_rom.xml());
	}

	Signal_handler<Main> _manual_deploy_handler {
		_env.ep(), *this, &Main::_handle_manual_deploy };


	/************
	 ** Global **
	 ************/

	Area _screen_size { };

	bool _leitzentrale_visible = false;

	Affinity::Space _affinity_space { 1, 1 };

	Sim_pin _sim_pin { };

	Modem_state _modem_state { };

	Current_call _current_call { };

	Dialed_number _dialed_number { };

	Registry<Registered<Section_dialog> > _section_dialogs { };

	/*
	 * Device section
	 */

	Device_section_dialog _device_section_dialog { _section_dialogs };

	/*
	 * Phone section
	 */

	Phone_section_dialog _phone_section_dialog { _section_dialogs, _modem_state };

	Conditional_float_dialog<Modem_power_dialog>
		_modem_power_dialog { "modempower", _modem_state, *this };

	Conditional_float_dialog<Pin_dialog>
		_pin_dialog { "pin", _sim_pin, *this };

	Conditional_float_dialog<Dialpad_dialog>
		_dialpad_dialog { "dialpad", _dialed_number, *this };

	Conditional_float_dialog<Current_call_dialog>
		_current_call_dialog { "call", _current_call, _dialed_number, *this };

	Conditional_float_dialog<Outbound_dialog>
		_outbound_dialog { "outbound", _modem_state, _current_call, *this };

	/*
	 * Storage section
	 */

	Storage_section_dialog _storage_section_dialog  { _section_dialogs };

	/*
	 * Network section
	 */

	Network_section_dialog _network_section_dialog  {
		_section_dialogs, _network._nic_target, _network._nic_state };

	/*
	 * Software section
	 */

	Software_section_dialog _software_section_dialog { _section_dialogs };

	Conditional_float_dialog<Graph>
		_graph { "graph",
		         _runtime_state, _cached_runtime_config, _storage._storage_devices,
		         _storage._sculpt_partition, _storage._ram_fs_state,
		         _popup.state, _deploy._children };

	/**
	 * Dialog interface
	 */
	Hover_result hover(Xml_node) override;

	void reset() override { }

	/**
	 * Dialog interface
	 */
	void generate(Xml_generator &xml) const override
	{
		xml.node("vbox", [&] {
			_device_section_dialog.generate(xml);

			_phone_section_dialog.generate(xml);
			_modem_power_dialog.generate_conditional(xml, _phone_section_dialog.selected());
			_pin_dialog.generate_conditional(xml, _phone_section_dialog.selected()
			                                   && _modem_state.ready()
			                                   && _modem_state.pin_required());

			_outbound_dialog.generate_conditional(xml, _phone_section_dialog.selected()
			                                        && _modem_state.ready()
			                                        && _modem_state.pin_ok());

			_dialpad_dialog.generate_conditional(xml, _phone_section_dialog.selected()
			                                       && _modem_state.ready()
			                                       && _modem_state.pin_ok());

			_current_call_dialog.generate_conditional(xml, _phone_section_dialog.selected()
			                                            && _modem_state.ready()
			                                            && _modem_state.pin_ok());

			_storage_section_dialog.generate(xml);

			_network_section_dialog.generate(xml);

			gen_named_node(xml, "float", "net settings", [&] {
				xml.attribute("east", "yes");
				xml.attribute("west", "yes");
				if (_network_section_dialog.selected())
					_network.dialog.generate(xml);
			});

			_software_section_dialog.generate(xml);

			_graph.generate_conditional(xml, _software_section_dialog.selected());
		});
	}

	/**
	 * Dialog::Generator interface
	 */
	void generate_dialog() override
	{
		_main_menu_view.generate();
	}

	Attached_rom_dataspace _runtime_state_rom { _env, "report -> runtime/state" };

	Runtime_state _runtime_state { _heap, _storage._sculpt_partition };

	Managed_config<Main> _runtime_config {
		_env, "config", "runtime", *this, &Main::_handle_runtime };

	bool _manually_managed_runtime = false;

	void _handle_runtime(Xml_node config)
	{
		_manually_managed_runtime = !config.has_type("empty");
		generate_runtime_config();
		generate_dialog();
	}

	void _generate_runtime_config(Xml_generator &) const;

	/**
	 * Runtime_config_generator interface
	 */
	void generate_runtime_config() override
	{
		if (!_runtime_config.try_generate_manually_managed())
			_runtime_config.generate([&] (Xml_generator &xml) {
				_generate_runtime_config(xml); });
	}

	Signal_handler<Main> _runtime_state_handler {
		_env.ep(), *this, &Main::_handle_runtime_state };

	void _handle_runtime_state();

	Attached_rom_dataspace const _platform { _env, "platform_info" };


	/****************************************
	 ** Cached model of the runtime config **
	 ****************************************/

	/*
	 * Even though the runtime configuration is generated by the sculpt
	 * manager, we still obtain it as a separate ROM session to keep the GUI
	 * part decoupled from the lower-level runtime configuration generator.
	 */
	Attached_rom_dataspace _runtime_config_rom { _env, "config -> managed/runtime" };

	Signal_handler<Main> _runtime_config_handler {
		_env.ep(), *this, &Main::_handle_runtime_config };

	Runtime_config _cached_runtime_config { _heap };

	void _handle_runtime_config()
	{
		_runtime_config_rom.update();
		_cached_runtime_config.update_from_xml(_runtime_config_rom.xml());
		generate_dialog(); /* update graph */
	}


	/****************************
	 ** Interactive operations **
	 ****************************/

	Constructible<Input::Seq_number> _clicked_seq_number { };
	Constructible<Input::Seq_number> _clacked_seq_number { };

	void _section_enabled(Section_dialog &section, bool enabled)
	{
		/* reset all sections to the default */
		_section_dialogs.for_each([&] (Section_dialog &dialog) {
			dialog.detail = Section_dialog::Detail::DEFAULT; });

		/* select specified section if enabled */
		bool any_selected = false;
		_section_dialogs.for_each([&] (Section_dialog &dialog) {
			if ((&dialog == &section) && enabled) {
				dialog.detail = Section_dialog::Detail::SELECTED;
				any_selected = true;
			}
		});

		/* minimize unselected sections if any section is selected */
		_section_dialogs.for_each([&] (Section_dialog &dialog) {
			if (dialog.detail != Section_dialog::Detail::SELECTED)
				dialog.detail = any_selected
				              ? Section_dialog::Detail::MINIMIZED
				              : Section_dialog::Detail::DEFAULT; });

		_main_menu_view.generate();
	}

	void _try_handle_click()
	{
		if (!_clicked_seq_number.constructed())
			return;

		Input::Seq_number const seq = *_clicked_seq_number;

		if (_main_menu_view.hovered(seq)) {

			/* determine clicked section */
			Section_dialog *clicked_ptr = nullptr;
			_section_dialogs.for_each([&] (Section_dialog &dialog) {
				if (dialog.hovered())
					clicked_ptr = &dialog; });

			/* toggle clicked section dialog */
			if (clicked_ptr)
				_section_enabled(*clicked_ptr, !clicked_ptr->selected());

			if (_modem_power_dialog.hovered())
				_modem_power_dialog.click();

			if (_pin_dialog.hovered())
				_pin_dialog.click();

			if (_dialpad_dialog.hovered())
				_dialpad_dialog.click();

			if (_outbound_dialog.hovered())
				_outbound_dialog.click();

			if (_current_call_dialog.hovered())
				_current_call_dialog.click();

			if (_graph.hovered())
				_graph.dialog.click(*this);

			if (_network.dialog.hovered)
				_network.dialog.click(_network);

			_main_menu_view.generate();
			_clicked_seq_number.destruct();
		}
	}

	void _try_handle_clack()
	{
		if (!_clacked_seq_number.constructed())
			return;

		Input::Seq_number const seq = *_clacked_seq_number;

		if (_main_menu_view.hovered(seq)) {

			_pin_dialog.clack();
			_dialpad_dialog.clack();
			_current_call_dialog.clack();

			if (_graph.hovered())
				_graph.dialog.clack(*this, _storage);

			_main_menu_view.generate();
			_clacked_seq_number.destruct();
		}
	}

	/**
	 * Menu_view::Hover_update_handler interface
	 */
	void menu_view_hover_updated() override
	{
		if (_clicked_seq_number.constructed())
			_try_handle_click();

		if (_clacked_seq_number.constructed())
			_try_handle_clack();
	}

	/* true while touched, used to issue only one click per touch sequence */
	bool _touched = false;

	/**
	 * Input_event_handler interface
	 */
	void handle_input_event(Input::Event const &ev) override
	{
		bool need_generate_dialog = false;

		if (ev.key_press(Input::BTN_LEFT) || ev.touch()) {
			if (!_touched) {
				_clicked_seq_number.construct(_global_input_seq_number);
				_try_handle_click();
				_touched = true;
			}
		}

		if (ev.key_release(Input::BTN_LEFT) || ev.touch_release()) {
			_clacked_seq_number.construct(_global_input_seq_number);
			_try_handle_clack();
			_touched = false;
		}

		if (need_generate_dialog)
			generate_dialog();
	}

	Menu_view _main_menu_view { _env, _child_states, *this, "menu_view",
	                             Ram_quota{4*1024*1024}, Cap_quota{150},
	                             "menu_dialog", "menu_view_hover", *this,
	                             Menu_view::Alpha::OPAQUE,
	                             Color { 62, 62, 67, 255 } };

	void _handle_window_layout();

	template <size_t N, typename FN>
	void _with_window(Xml_node window_list, String<N> const &match, FN const &fn)
	{
		window_list.for_each_sub_node("window", [&] (Xml_node win) {
			if (win.attribute_value("label", String<N>()) == match)
				fn(win); });
	}

	Attached_rom_dataspace _window_list { _env, "window_list" };

	Signal_handler<Main> _window_list_handler {
		_env.ep(), *this, &Main::_handle_window_layout };

	Expanding_reporter _wm_focus { _env, "focus", "wm_focus" };

	Attached_rom_dataspace _decorator_margins { _env, "decorator_margins" };

	Signal_handler<Main> _decorator_margins_handler {
		_env.ep(), *this, &Main::_handle_window_layout };

	Expanding_reporter _window_layout { _env, "window_layout", "window_layout" };

	/*
	 * Fs_dialog::Action interface
	 */
	void toggle_inspect_view(Storage_target const &) override { }

	void use(Storage_target const &target) override { _storage.use(target); }

	/*
	 * Storage_dialog::Action interface
	 */
	void format(Storage_target const &target) override
	{
		_storage.format(target);
	}

	void cancel_format(Storage_target const &target) override
	{
		_storage.cancel_format(target);
		_graph.dialog.reset_storage_operation();
	}

	void expand(Storage_target const &target) override
	{
		_storage.expand(target);
	}

	void cancel_expand(Storage_target const &target) override
	{
		_storage.cancel_expand(target);
		_graph.dialog.reset_storage_operation();
	}

	void check(Storage_target const &target) override
	{
		_storage.check(target);
	}

	void toggle_default_storage_target(Storage_target const &target) override
	{
		_storage.toggle_default_storage_target(target);
	}

	/*
	 * Graph::Action interface
	 */
	void remove_deployed_component(Start_name const &name) override
	{
		_runtime_state.abandon(name);

		/* update config/managed/deploy with the component 'name' removed */
		_deploy.update_managed_deploy_config(_manual_deploy_rom.xml());
	}

	/*
	 * Graph::Action interface
	 */
	void restart_deployed_component(Start_name const &name) override
	{
		if (name == "nic_drv") {

			_network.restart_nic_drv_on_next_runtime_cfg();
			generate_runtime_config();

		} else if (name == "wifi_drv") {

			_network.restart_wifi_drv_on_next_runtime_cfg();
			generate_runtime_config();

		} else if (name == "usb_net") {

			_network.restart_usb_net_on_next_runtime_cfg();
			generate_runtime_config();

		} else {

			_runtime_state.restart(name);

			/* update config/managed/deploy with the component 'name' removed */
			_deploy.update_managed_deploy_config(_manual_deploy_rom.xml());
		}
	}

	/*
	 * Graph::Action interface
	 */
	void toggle_launcher_selector(Rect) override { }


	/***********
	 ** Audio **
	 ***********/

	Expanding_reporter _audio_config { _env, "config", "audio_config" };

	struct Audio_config
	{
		bool earpiece, speaker, mic, modem;

		bool operator != (Audio_config const &other) const
		{
			return (earpiece != other.earpiece)
			    || (speaker  != other.speaker)
			    || (mic      != other.mic)
			    || (modem    != other.modem);
		}

		void generate(Xml_generator &xml) const
		{
			xml.node("earpiece", [&] () {
				xml.attribute("volume", earpiece ? 100 : 0);
			});
			xml.node("speaker", [&] () {
				xml.attribute("volume", speaker ? 100 : 0);
			});
			xml.node("mic", [&] () {
				xml.attribute("volume", mic ? 80 : 0);
			});
			xml.node("codec", [&]() {
				xml.attribute("target", modem ? "modem" : "soc");
			});
		}
	};

	Audio_config _curr_audio_config { };

	void _generate_audio_config()
	{
		Audio_config const new_config {

			.earpiece = true,

			/* enable speaker for the ring tone when no call is active */
			.speaker  = !_current_call.active() || _current_call.speaker,

			/* enable microphone during call */
			.mic = _current_call.active(),

			/* set codec target during call */
			.modem = _current_call.active()
		};

		if (new_config != _curr_audio_config) {
			_curr_audio_config = new_config;
			_audio_config.generate([&] (Xml_generator &xml) {
				_curr_audio_config.generate(xml); });
		}
	}


	/***********
	 ** Phone **
	 ***********/

	Expanding_reporter _modem_config { _env, "config", "modem_config" };

	enum class Modem_config_power { ANY, OFF, ON };

	Modem_config_power _modem_config_power { Modem_config_power::ANY };

	/*
	 * State that influences the modem configuration, used to detect the
	 * need for configuraton updates.
	 */
	struct Modem_config
	{
		Modem_config_power modem_power;
		Modem_state        modem_state;
		Sim_pin            sim_pin;
		Current_call       current_call;

		bool operator != (Modem_config const &other) const
		{
			return (modem_power  != other.modem_power)
			    || (modem_state  != other.modem_state)
			    || (sim_pin      != other.sim_pin)
			    || (current_call != other.current_call);
		}

		void generate(Xml_generator &xml) const
		{
			switch (modem_power) {
			case Modem_config_power::OFF: xml.attribute("power", "off"); break;
			case Modem_config_power::ON:  xml.attribute("power", "on");  break;
			case Modem_config_power::ANY: break;
			}

			bool const supply_pin = modem_state.pin_required()
			                     && sim_pin.suitable_for_unlock()
			                     && sim_pin.confirmed;
			if (supply_pin)
				xml.attribute("pin", String<10>(sim_pin));

			xml.node("ring", [&] {
				xml.append_content("AT+QLDTMF=5,\"4,3,6,#,D,3\",1"); });

			current_call.gen_modem_config(xml);
		}
	};

	Modem_config _curr_modem_config { };

	Attached_rom_dataspace _modem_state_rom { _env, "report -> drivers/modem/state" };

	Signal_handler<Main> _modem_state_handler {
		_env.ep(), *this, &Main::_handle_modem_state };

	void _handle_modem_state()
	{
		_modem_state_rom.update();

		if (_verbose_modem)
			log("modem state: ", _modem_state_rom.xml());

		Modem_state const orig_modem_state = _modem_state;

		_modem_state = Modem_state::from_xml(_modem_state_rom.xml());

		_current_call.update(_modem_state);

		if (_modem_state.pin_rejected())
			_sim_pin = Sim_pin { };

		bool const configured_current_call_out_of_date =
			(_current_call != _curr_modem_config.current_call);

		bool const modem_state_changed = (orig_modem_state != _modem_state);

		if (configured_current_call_out_of_date || modem_state_changed) {
			_generate_modem_config();
			generate_dialog();
		}
	}

	void _generate_modem_config()
	{
		Modem_config const new_config {
			.modem_power  = _modem_config_power,
			.modem_state  = _modem_state,
			.sim_pin      = _sim_pin,
			.current_call = _current_call,
		};

		if (new_config != _curr_modem_config) {

			_curr_modem_config = new_config;

			_modem_config.generate([&] (Xml_generator &xml) {

				if (_verbose_modem)
					xml.attribute("verbose", "yes");

				_curr_modem_config.generate(xml);
			});
		}

		/* update audio config as it depends on the current call state */
		_generate_audio_config();
	}

	/**
	 * Modem_power_dialog::Action interface
	 */
	void modem_power(bool enabled) override
	{
		_modem_config_power = enabled ? Modem_config_power::ON
		                              : Modem_config_power::OFF;

		/* forget pin and call state when powering off the modem */
		if (!enabled) {
			_sim_pin      = { };
			_current_call = { };
		}

		_generate_modem_config();
	}

	/**
	 * Pin_dialog::Action interface
	 */
	void append_sim_pin_digit(Sim_pin::Digit d) override
	{
		_sim_pin.append_digit(d);
	}

	/**
	 * Pin_dialog::Action interface
	 */
	void remove_last_sim_pin_digit() override
	{
		_sim_pin.remove_last_digit();
	}

	/**
	 * Pin_dialog::Action interface
	 */
	void confirm_sim_pin() override
	{
		if (_sim_pin.suitable_for_unlock())
			_sim_pin.confirmed = true;
		_generate_modem_config();
	}

	/**
	 * Dialpad_dialog::Action interface
	 */
	void append_dial_digit(Dialed_number::Digit d) override
	{
		_dialed_number.append_digit(d);
	}

	/**
	 * Dialpad_dialog::Action interface
	 */
	void remove_last_dial_digit() override
	{
		_dialed_number.remove_last_digit();
	}

	/**
	 * Current_call_dialog::Action interface
	 */
	void accept_incoming_call() override
	{
		_current_call.accept();
		_generate_modem_config();
	}

	/**
	 * Current_call_dialog::Action interface
	 */
	void reject_incoming_call() override
	{
		_current_call.reject();
		_generate_modem_config();
	}

	/**
	 * Current_call_dialog::Action interface
	 */
	void hang_up() override
	{
		_current_call.reject();
		_generate_modem_config();
	}

	/**
	 * Current_call_dialog::Action interface
	 */
	void toggle_speaker() override
	{
		_current_call.toggle_speaker();
		_generate_modem_config();
	}

	/**
	 * Current_call_dialog::Action interface
	 */
	void initiate_call() override
	{
		if (_dialed_number.suitable_for_call()) {
			_current_call.initiate(Number(_dialed_number));
			_generate_modem_config();
		}
	}

	/**
	 * Current_call_dialog::Action interface
	 */
	void cancel_initiated_call() override
	{
		_current_call.cancel();
		_generate_modem_config();
	}


	/*******************
	 ** Runtime graph **
	 *******************/

	Popup _popup { };

	Main(Env &env) : _env(env)
	{
		/* XXX select phone dialog by default */
		_section_enabled(_phone_section_dialog, true);

		_config.sigh(_config_handler);
		_leitzentrale_rom.sigh(_leitzentrale_handler);
		_manual_deploy_rom.sigh(_manual_deploy_handler);
		_runtime_state_rom.sigh(_runtime_state_handler);
		_runtime_config_rom.sigh(_runtime_config_handler);
		_gui.input()->sigh(_input_handler);
		_gui.mode_sigh(_gui_mode_handler);
		_launcher_listing_rom.sigh(_launcher_listing_handler);

		/*
		 * Subscribe to reports
		 */
		_update_state_rom .sigh(_update_state_handler);
		_window_list      .sigh(_window_list_handler);
		_decorator_margins.sigh(_decorator_margins_handler);
		_modem_state_rom  .sigh(_modem_state_handler);
		_blueprint_rom    .sigh(_blueprint_handler);

		/*
		 * Import initial report content
		 */
		_handle_config();
		_handle_leitzentrale();
		_handle_gui_mode();
		_storage.handle_storage_devices_update();
		_handle_runtime_config();
		_handle_modem_state();

		/*
		 * Read static platform information
		 */
		_platform.xml().with_optional_sub_node("affinity-space", [&] (Xml_node const &node) {
			_affinity_space = Affinity::Space(node.attribute_value("width",  1U),
			                                  node.attribute_value("height", 1U));
		});

		/*
		 * Generate initial config/managed/deploy configuration
		 */
		_handle_manual_deploy();

		_generate_modem_config();
		generate_runtime_config();
		generate_dialog();
	}
};


void Sculpt::Main::_handle_window_layout()
{
	/* skip window-layout handling (and decorator activity) while booting */
	if (!_gui_mode_ready)
		return;

	struct Decorator_margins
	{
		unsigned top = 0, bottom = 0, left = 0, right = 0;

		Decorator_margins(Xml_node node)
		{
			if (!node.has_sub_node("floating"))
				return;

			Xml_node const floating = node.sub_node("floating");

			top    = floating.attribute_value("top",    0U);
			bottom = floating.attribute_value("bottom", 0U);
			left   = floating.attribute_value("left",   0U);
			right  = floating.attribute_value("right",  0U);
		}
	};

	/* read decorator margins from the decorator's report */
	_decorator_margins.update();
	Decorator_margins const margins(_decorator_margins.xml());

	typedef String<128> Label;
	Label const menu_view_label("runtime -> leitzentrale -> menu_view");

	_window_list.update();
	Xml_node const window_list = _window_list.xml();

	auto win_size = [&] (Xml_node win) {
		return Area(win.attribute_value("width",  0U),
		            win.attribute_value("height", 0U)); };

	Framebuffer::Mode const mode = _gui.mode();

	/* suppress intermediate boot-time states before the framebuffer driver is up */
	if (mode.area.count() <= 1)
		return;

	_window_layout.generate([&] (Xml_generator &xml) {

		auto gen_window = [&] (Xml_node win, Rect rect) {
			if (rect.valid()) {
				xml.node("window", [&] () {
					xml.attribute("id",     win.attribute_value("id", 0UL));
					xml.attribute("xpos",   rect.x1());
					xml.attribute("ypos",   rect.y1());
					xml.attribute("width",  rect.w());
					xml.attribute("height", rect.h());
					xml.attribute("title",  win.attribute_value("label", Label()));
				});
			}
		};

		_with_window(window_list, menu_view_label, [&] (Xml_node win) {
			Area  const size = win_size(win);
			Point const pos(_leitzentrale_visible ? 0 : (int)size.w(), 0);
			gen_window(win, Rect(pos, size));
		});
	});
}


void Sculpt::Main::_handle_gui_mode()
{
	Framebuffer::Mode const mode = _gui.mode();

	if (mode.area.count() > 1)
		_gui_mode_ready = true;

	_handle_window_layout();

	_screen_size = mode.area;
	_main_menu_view.min_width  = _screen_size.w();
	_main_menu_view.min_height = _screen_size.h();

	generate_runtime_config();
}


Sculpt::Dialog::Hover_result Sculpt::Main::hover(Xml_node hover)
{
	_section_dialogs.for_each([&] (Section_dialog &dialog) {
		dialog.hover(Xml_node("<empty/>")); });

	hover.with_optional_sub_node("vbox", [&] (Xml_node const &vbox) {
		_section_dialogs.for_each([&] (Section_dialog &dialog) {
			dialog.hover(vbox); });

		_modem_power_dialog .hover(vbox);
		_pin_dialog         .hover(vbox);
		_dialpad_dialog     .hover(vbox);
		_current_call_dialog.hover(vbox);
		_outbound_dialog    .hover(vbox);
		_graph              .hover(vbox);

		_network.dialog.match_sub_dialog(vbox, "float");
	});

	return Dialog::Hover_result { };
}


void Sculpt::Main::_handle_update_state()
{
	_update_state_rom.update();
	generate_dialog();

	Xml_node const update_state = _update_state_rom.xml();

	if (update_state.num_sub_nodes() == 0)
		return;

	_download_queue.apply_update_state(update_state);
	_download_queue.remove_inactive_downloads();

	bool const installation_complete =
		!update_state.attribute_value("progress", false);

	if (installation_complete) {

		Xml_node const blueprint = _blueprint_rom.xml();
		bool const new_depot_query_needed = blueprint_any_missing(blueprint)
		                                 || blueprint_any_rom_missing(blueprint);
		if (new_depot_query_needed)
			trigger_depot_query();

		_deploy.reattempt_after_installation();
	}
}


void Sculpt::Main::_handle_runtime_state()
{
	_runtime_state_rom.update();

	Xml_node state = _runtime_state_rom.xml();

	_runtime_state.update_from_state_report(state);

	bool reconfigure_runtime = false;
	bool regenerate_dialog   = false;

	/* check for completed storage operations */
	_storage._storage_devices.for_each([&] (Storage_device &device) {

		device.for_each_partition([&] (Partition &partition) {

			Storage_target const target { device.label, partition.number };

			if (partition.check_in_progress) {
				String<64> name(target.label(), ".e2fsck");
				Child_exit_state exit_state(state, name);

				if (exit_state.exited) {
					if (exit_state.code != 0)
						error("file-system check failed");
					if (exit_state.code == 0)
						log("file-system check succeeded");

					partition.check_in_progress = 0;
					reconfigure_runtime = true;
					_storage.dialog.reset_operation();
					_graph.dialog.reset_storage_operation();
				}
			}

			if (partition.format_in_progress) {
				String<64> name(target.label(), ".mke2fs");
				Child_exit_state exit_state(state, name);

				if (exit_state.exited) {
					if (exit_state.code != 0)
						error("file-system creation failed");

					partition.format_in_progress = false;
					partition.file_system.type = File_system::EXT2;

					if (partition.whole_device())
						device.rediscover();

					reconfigure_runtime = true;
					_storage.dialog.reset_operation();
					_graph.dialog.reset_storage_operation();
				}
			}

			/* respond to completion of file-system resize operation */
			if (partition.fs_resize_in_progress) {
				Child_exit_state exit_state(state, Start_name(target.label(), ".resize2fs"));
				if (exit_state.exited) {
					partition.fs_resize_in_progress = false;
					reconfigure_runtime = true;
					device.rediscover();
					_storage.dialog.reset_operation();
					_graph.dialog.reset_storage_operation();
				}
			}

		}); /* for each partition */

		/* respond to failure of part_block */
		if (device.discovery_in_progress()) {
			Child_exit_state exit_state(state, device.part_block_start_name());
			if (!exit_state.responsive) {
				error(device.part_block_start_name(), " got stuck");
				device.state = Storage_device::RELEASED;
				reconfigure_runtime = true;
			}
		}

		/* respond to completion of GPT relabeling */
		if (device.relabel_in_progress()) {
			Child_exit_state exit_state(state, device.relabel_start_name());
			if (exit_state.exited) {
				device.rediscover();
				reconfigure_runtime = true;
				_storage.dialog.reset_operation();
				_graph.dialog.reset_storage_operation();
			}
		}

		/* respond to completion of GPT expand */
		if (device.gpt_expand_in_progress()) {
			Child_exit_state exit_state(state, device.expand_start_name());
			if (exit_state.exited) {

				/* kick off resize2fs */
				device.for_each_partition([&] (Partition &partition) {
					if (partition.gpt_expand_in_progress) {
						partition.gpt_expand_in_progress = false;
						partition.fs_resize_in_progress  = true;
					}
				});

				reconfigure_runtime = true;
				_storage.dialog.reset_operation();
				_graph.dialog.reset_storage_operation();
			}
		}

	}); /* for each device */

	/* handle failed initialization of USB-storage devices */
	_storage._storage_devices.usb_storage_devices.for_each([&] (Usb_storage_device &dev) {
		String<64> name(dev.usb_block_drv_name());
		Child_exit_state exit_state(state, name);
		if (exit_state.exited) {
			dev.discard_usb_block_drv();
			reconfigure_runtime = true;
			regenerate_dialog   = true;
		}
	});

	/* remove prepare subsystem when finished */
	{
		Child_exit_state exit_state(state, "prepare");
		if (exit_state.exited) {
			_prepare_completed = _prepare_version;

			/* trigger update and deploy */
			reconfigure_runtime = true;
		}
	}

	/* schedule pending file operations to new fs_tool instance */
	{
		Child_exit_state exit_state(state, "fs_tool");

		if (exit_state.exited) {

			Child_exit_state::Version const expected_version(_fs_tool_version.value);

			if (exit_state.version == expected_version) {

				_file_operation_queue.schedule_next_operations();
				_fs_tool_version.value++;
				reconfigure_runtime = true;
			}
		}
	}

	/* upgrade RAM and cap quota on demand */
	state.for_each_sub_node("child", [&] (Xml_node child) {

		bool reconfiguration_needed = false;
		_child_states.for_each([&] (Child_state &child_state) {
			if (child_state.apply_child_state_report(child))
				reconfiguration_needed = true; });

		if (reconfiguration_needed) {
			reconfigure_runtime = true;
			regenerate_dialog   = true;
		}
	});

	if (_deploy.update_child_conditions()) {
		reconfigure_runtime = true;
		regenerate_dialog   = true;
	}

	if (regenerate_dialog)
		generate_dialog();

	if (reconfigure_runtime)
		generate_runtime_config();
}


void Sculpt::Main::_generate_runtime_config(Xml_generator &xml) const
{
	xml.attribute("verbose", "yes");

	xml.attribute("prio_levels", _prio_levels.value);

	xml.node("report", [&] () {
		xml.attribute("init_ram",   "yes");
		xml.attribute("init_caps",  "yes");
		xml.attribute("child_ram",  "yes");
		xml.attribute("child_caps", "yes");
		xml.attribute("delay_ms",   4*500);
		xml.attribute("buffer",     "1M");
	});

	xml.node("heartbeat", [&] () { xml.attribute("rate_ms", 2000); });

	xml.node("parent-provides", [&] () {
		gen_parent_service<Rom_session>(xml);
		gen_parent_service<Cpu_session>(xml);
		gen_parent_service<Pd_session>(xml);
		gen_parent_service<Rm_session>(xml);
		gen_parent_service<Log_session>(xml);
		gen_parent_service<Vm_session>(xml);
		gen_parent_service<Timer::Session>(xml);
		gen_parent_service<Report::Session>(xml);
		gen_parent_service<Platform::Session>(xml);
		gen_parent_service<Block::Session>(xml);
		gen_parent_service<Usb::Session>(xml);
		gen_parent_service<::File_system::Session>(xml);
		gen_parent_service<Gui::Session>(xml);
		gen_parent_service<Rtc::Session>(xml);
		gen_parent_service<Trace::Session>(xml);
		gen_parent_service<Io_mem_session>(xml);
		gen_parent_service<Io_port_session>(xml);
		gen_parent_service<Irq_session>(xml);
		gen_parent_service<Event::Session>(xml);
		gen_parent_service<Capture::Session>(xml);
		gen_parent_service<Gpu::Session>(xml);
		gen_parent_service<Pin_state::Session>(xml);
		gen_parent_service<Pin_control::Session>(xml);
	});

	xml.node("affinity-space", [&] () {
		xml.attribute("width",  _affinity_space.width());
		xml.attribute("height", _affinity_space.height());
	});

	_main_menu_view.gen_start_node(xml);
	_storage.gen_runtime_start_nodes(xml);

	/*
	 * Load configuration and update depot config on the sculpt partition
	 */
	if (_storage._sculpt_partition.valid() && _prepare_in_progress())
		xml.node("start", [&] () {
			gen_prepare_start_content(xml, _prepare_version); });

	/*
	 * Spawn chroot instances for accessing '/depot' and '/public'. The
	 * chroot instances implicitly refer to the 'default_fs_rw'.
	 */
	if (_storage._sculpt_partition.valid()) {

		auto chroot = [&] (Start_name const &name, Path const &path, Writeable w) {
			xml.node("start", [&] () {
				gen_chroot_start_content(xml, name, path, w); }); };

		if (_update_running()) {
			chroot("depot_rw",  "/depot",  WRITEABLE);
			chroot("public_rw", "/public", WRITEABLE);
		}

		chroot("depot", "/depot",  READ_ONLY);
	}

	/* execute file operations */
	if (_storage._sculpt_partition.valid())
		if (_file_operation_queue.any_operation_in_progress())
			xml.node("start", [&] () {
				gen_fs_tool_start_content(xml, _fs_tool_version,
				                          _file_operation_queue); });

	_network.gen_runtime_start_nodes(xml);

	if (_update_running())
		xml.node("start", [&] () {
			gen_update_start_content(xml); });

	if (_storage._sculpt_partition.valid() && !_prepare_in_progress()) {
		xml.node("start", [&] () {
			gen_launcher_query_start_content(xml); });

		_deploy.gen_runtime_start_nodes(xml, _prio_levels, _affinity_space);
	}
}


void Component::construct(Genode::Env &env)
{
	static Sculpt::Main main(env);
}

