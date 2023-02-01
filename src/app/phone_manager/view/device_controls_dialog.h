/*
 * \brief  Device controls (e.g., brightness) dialog
 * \author Norman Feske
 * \date   2022-11-22
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__DEVICE_CONTROLS_DIALOG_H_
#define _VIEW__DEVICE_CONTROLS_DIALOG_H_

#include <view/dialog.h>
#include <view/hoverable_item.h>
#include <model/power_state.h>
#include <model/mic_state.h>
#include <model/audio_volume.h>

namespace Sculpt { struct Device_controls_dialog; }


struct Sculpt::Device_controls_dialog
{
	Power_state  const &_power_state;
	Mic_state    const &_mic_state;
	Audio_volume const &_audio_volume;

	struct Action : Interface
	{
		virtual void select_brightness_level(unsigned) = 0;

		virtual void select_volume_level(unsigned) = 0;

		virtual void select_mic_policy(Mic_state const &) = 0;
	};

	Action &_action;

	Device_controls_dialog(Power_state  const &power_state,
	                       Mic_state    const &mic_state,
	                       Audio_volume const &audio_volume,
	                       Action &action)
	:
		_power_state(power_state), _mic_state(mic_state),
		_audio_volume(audio_volume),_action(action)
	{ }

	using Hover_result = Hoverable_item::Hover_result;

	using Id = Hoverable_item::Id;


	/**********************************
	 ** Brightness and volume levels **
	 **********************************/

	Hoverable_item _control { };
	Hoverable_item _level { };

	bool _dragged = false;

	void _gen_level(Xml_generator &xml, unsigned const percent,
	                char const *control_id, char const *text) const
	{
		gen_named_node(xml, "frame", control_id, [&] {

			xml.attribute("style", "important");

			gen_named_node(xml, "float", "label", [&] {
				xml.attribute("west", "yes");
				gen_named_node(xml, "label", "label", [&] {
					xml.attribute("text", String<30>("  ", text));
					xml.attribute("min_ex", "15");
				});
			});

			gen_named_node(xml, "float", "onoff", [&] {
				xml.attribute("east", "yes");
				gen_named_node(xml, "hbox", "range", [&] {

					for (unsigned i = 0; i < 10; i++) {

						gen_named_node(xml, "button", Id(i), [&] {

							if (i*10 <= percent)
								xml.attribute("selected", "yes");
							else
								xml.attribute("style", "unimportant");

							xml.node("label", [&] {
								xml.attribute("text", " "); });
						});
					}
				});
			});
		});
	}

	void _select_brightness_or_volume_level()
	{
		unsigned value = 0;
		if (!ascii_to(_level._hovered.string(), value))
			return;

		unsigned const percent = max(10u, min(100u, value*10 + 9));

		if (_control.hovered("brightness"))
			_action.select_brightness_level(percent);

		if (_control.hovered("volume"))
			_action.select_volume_level(percent);
	}


	/****************
	 ** Mic policy **
	 ****************/

	Hoverable_item _mic_choice { };

	void _gen_mic_policy(Xml_generator &xml) const
	{
		gen_named_node(xml, "frame", "mic", [&] {

			xml.attribute("style", "important");

			gen_named_node(xml, "float", "label", [&] {
				xml.attribute("west", "yes");
				gen_named_node(xml, "label", "label", [&] {
					xml.attribute("text", "  Microphone");
					xml.attribute("min_ex", "15");
				});
			});

			gen_named_node(xml, "float", "onoff", [&] {
				xml.attribute("east", "yes");
				gen_named_node(xml, "hbox", "onoff", [&] {

					auto gen_option = [&] (auto id, bool selected, auto text)
					{
						gen_named_node(xml, "button", id, [&] {

							if (selected)
								xml.attribute("selected", "yes");

							xml.node("label", [&] {
								xml.attribute("text", text); });
						});
					};

					gen_option("off",   (_mic_state == Mic_state::OFF),   "  Off  ");
					gen_option("phone", (_mic_state == Mic_state::PHONE), "  Phone  ");
					gen_option("on",    (_mic_state == Mic_state::ON),    "  On  ");
				});
			});
		});
	}


	void generate(Xml_generator &xml) const
	{
		auto gen_space = [&] (auto const &id)
		{
			gen_named_node(xml, "label", id, [&] {
				xml.attribute("text", " "); });
		};

		gen_named_node(xml, "vbox", "vbox", [&] {
			_gen_level(xml, _power_state.brightness, "brightness", "Brightness");
			gen_space("below brightness");
			_gen_level(xml, _audio_volume.value,     "volume",     "Volume");
			gen_space("below volume");
			_gen_mic_policy(xml);
		});
	}

	Hover_result hover(Xml_node hover)
	{
		Hover_result const level_result =
			_level.match(hover, "vbox", "frame", "float", "hbox", "button", "name");

		Hover_result const control_result =
			_control.match(hover, "vbox", "frame", "name");

		Hover_result const mic_result =
			_mic_choice.match(hover, "vbox", "frame", "float", "hbox", "button", "name");

		if (_dragged)
			_select_brightness_or_volume_level();

		return Dialog::any_hover_changed(level_result, control_result, mic_result);
	}

	bool hovered() const { return _level._hovered.valid(); }

	void click()
	{
		_dragged = true;
		_select_brightness_or_volume_level();

		if (_control.hovered("mic")) {
			if (_mic_choice.hovered("off"))   _action.select_mic_policy(Mic_state::OFF);
			if (_mic_choice.hovered("phone")) _action.select_mic_policy(Mic_state::PHONE);
			if (_mic_choice.hovered("on"))    _action.select_mic_policy(Mic_state::ON);
		}
	}

	void clack()
	{
		_dragged = false;
	}
};

#endif /* _VIEW__DEVICE_CONTROLS_DIALOG_H_ */
