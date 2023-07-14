/*
 * \brief  Dialog for interacting with the current voice call
 * \author Norman Feske
 * \date   2022-06-30
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__CURRENT_CALL_DIALOG_H_
#define _VIEW__CURRENT_CALL_DIALOG_H_

#include <view/dialog.h>
#include <model/current_call.h>

namespace Sculpt { struct Current_call_dialog; }


struct Sculpt::Current_call_dialog
{
	Current_call  const &_current_call;
	Dialed_number const &_dialed_number;

	Hoverable_item::Id _clicked { };

	struct Action : Interface
	{
		using Number = String<128>;

		virtual void accept_incoming_call() = 0;
		virtual void reject_incoming_call() = 0;
		virtual void hang_up() = 0;
		virtual void toggle_speaker() = 0;
		virtual void initiate_call() = 0;
		virtual void cancel_initiated_call() = 0;
		virtual void remove_last_dial_digit() = 0;
	};

	Action &_action;

	using Hover_result = Hoverable_item::Hover_result;

	Hoverable_item _button { };

	Current_call_dialog(Current_call  const &current_call,
	                    Dialed_number const &dialed_number,
	                    Action &action)
	:
		_current_call(current_call), _dialed_number(dialed_number), _action(action)
	{ }

	void _gen_button(Xml_generator &xml, char const *id, bool selected, String<64> const &text) const
	{
		gen_named_node(xml, "button", id, [&] {

			if (selected)
				xml.attribute("selected", "yes");

			xml.node("label", [&] {
				xml.attribute("text", text); });
		});
	}

	void _gen_active_call(Xml_generator &xml) const
	{
		auto message = [] (Current_call::State call_state)
		{
			using State = Current_call::State;

			switch (call_state) {
			case State::NONE:      break;
			case State::INCOMING:  return " Call from ";
			case State::ACCEPTED:  return " Connected from ";
			case State::REJECTED:  return " Disconnecting from ";
			case State::HUNG_UP:   return " Disconnected from ";
			case State::INITIATED: return " Dialing ";
			case State::OUTBOUND:  return " Connecting to ";
			case State::ALERTING:  return " Alerting ";
			case State::ACTIVE:    return " Connected to ";
			case State::CANCELED:  return " Canceled call to ";
			}
			return " Failed "; /* never reached */
		};

		gen_named_node(xml, "vbox", "message", [&] {

			gen_named_node(xml, "label", "label", [&] {
				xml.attribute("text", message(_current_call.state)); });

			gen_named_node(xml, "label", "number", [&] {
				xml.attribute("text", String<128>(" ", _current_call.number, " "));
				xml.attribute("min_ex", "15");
			});
		});

		gen_named_node(xml, "float", "choice", [&] {
			xml.attribute("east", "yes");
			gen_named_node(xml, "hbox", "choice", [&] {

				if (_current_call.incoming()) {
					_gen_button(xml, "accept", _clicked == "accept", " Accept ");
					_gen_button(xml, "reject", _clicked == "reject", " Reject ");
				}
				if (_current_call.connecting()) {
					_gen_button(xml, "cancel", _clicked == "cancel", " Cancel ");
				}
				if (_current_call.accepted() || _current_call.active()) {
					_gen_button(xml, "speaker", _current_call.speaker, " Speaker ");
					_gen_button(xml, "hang up", _clicked == "hang up", " Hang up ");
				}
			});
		});
	}

	void _gen_call_operations(Xml_generator &xml) const
	{
		auto gen_vspace = [&] (auto name)
		{
			gen_named_node(xml, "vbox", name, [&] {
				gen_named_node(xml, "label", "first", [&] {
						xml.attribute("text", " "); });
					gen_named_node(xml, "label", "second", [&] {
						xml.attribute("text", " "); }); });
		};

		gen_vspace("left");
		gen_named_node(xml, "float", "center", [&] {
			xml.node("hbox", [&] {

				gen_named_node(xml, "button", "clear", [&] {
					if (!_dialed_number.at_least_one_digit())
						xml.attribute("style", "unimportant");
					else if (_clicked == "clear")
						xml.attribute("selected", "yes");
					xml.node("label", [&] {
						xml.attribute("text", " Clear "); });
				});

				gen_named_node(xml, "label", "hspace", [&] {
					xml.attribute("text", "");
					xml.attribute("min_ex", 2); });

				gen_named_node(xml, "button", "initiate", [&] {
					if (!_dialed_number.suitable_for_call())
						xml.attribute("style", "unimportant");
					else if (_clicked == "initiate")
						xml.attribute("selected", "yes");
					xml.node("label", [&] {
						xml.attribute("text", " Initiate Call "); });
				});
			});
		});
		gen_vspace("right");
	}

	void generate(Xml_generator &xml) const
	{
		gen_named_node(xml, "frame", "call", [&] {

			if (_current_call.none()) {
				xml.attribute("style", "invisible");
				xml.node("hbox", [&] {

					_gen_call_operations(xml);
				});

			} else {

				xml.attribute("style", "transient");
				xml.node("hbox", [&] {
					_gen_active_call(xml); });
			}
		});
	}

	Hover_result hover(Xml_node hover)
	{
		return _button.match(hover, "frame", "hbox", "float", "hbox", "button", "name");
	}

	bool hovered() const { return _button._hovered.valid(); }

	void click()
	{
		_clicked = _button._hovered;

		if (_button.hovered("reject"))   _action.reject_incoming_call();
		if (_button.hovered("accept"))   _action.accept_incoming_call();
		if (_button.hovered("speaker"))  _action.toggle_speaker();
		if (_button.hovered("hang up"))  _action.hang_up();
		if (_button.hovered("initiate")) _action.initiate_call();
		if (_button.hovered("cancel"))   _action.cancel_initiated_call();
		if (_button.hovered("clear"))    _action.remove_last_dial_digit();
	}

	void clack() { _clicked = Hoverable_item::Id(); }
};

#endif /* _VIEW__CURRENT_CALL_DIALOG_H_ */
