/*
 * \brief  AT-protocol handling
 * \author Norman Feske
 * \date   2022-06-15
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _AT_PROTOCOL__CONTROL_H_
#define _AT_PROTOCOL__CONTROL_H_

/* Genode includes */
#include <util/xml_node.h>

/* AT-protocol includes */
#include <at_protocol/types.h>
#include <at_protocol/status.h>

namespace At_protocol { struct Control; }


class At_protocol::Control : Noncopyable
{
	private:

		using Pin   = String<8>;
		using Power = String<16>;

		Status const &_status;

		struct Operation : Interface, Noncopyable
		{
			enum class State { PENDING, SUBMITTED, TIMED_OUT };
			State state = State::PENDING;
			virtual Command command() const = 0;
		};

		struct Check_ready : Operation
		{
			Command command() const override { return AT_CHECK_READY; };
		};

		struct Disable_echo : Operation
		{
			Command command() const override { return "ATE0"; };
		};

		struct Request_pin_state : Operation
		{
			Command command() const override { return "AT+CPIN?"; };
		};

		struct Set_pin : Operation
		{
			Pin const pin;

			Command command() const override
			{
				return Command("AT+CPIN=", pin);
			}

			Set_pin(Pin const &pin) : pin(pin) { }
		};

		struct Request_pin_count : Operation
		{
			Command command() const override { return "AT+QPINC?"; }
		};

		struct Request_call_list : Operation
		{
			Command command() const override { return AT_REQUEST_CALL_LIST; }
		};

		struct Initiate_call : Operation
		{
			Number const number;

			Command command() const override
			{
				return Command("ATD", number, ";");
			};

			Initiate_call(Number const &number) : number(number) { }
		};

		struct Accept_call : Operation
		{
			Command command() const override { return AT_ACCEPT_CALL; };
		};

		struct Hang_up : Operation
		{
			Command command() const override { return AT_HANG_UP; };
		};

		struct Power_down : Operation
		{
			Command command() const override { return AT_POWER_DOWN; };
		};

		struct Reboot : Operation
		{
			Command command() const override { return Command("AT+CFUN=1,1"); }
		};

		struct Query_qcfg : Operation
		{
			Qcfg::Name const name;

			Command command() const override
			{
				return Command(AT_QCFG_PREFIX, "\"", name, "\"");
			}

			Query_qcfg(Qcfg::Name const &name) : name(name) { }
		};

		struct Assign_qcfg : Operation
		{
			Qcfg::Name  const name;
			Qcfg::Value const value;

			Command command() const override
			{
				return Command(AT_QCFG_PREFIX, "\"", name, "\",", value);
			}

			Assign_qcfg(Qcfg::Name  const &name, Qcfg::Value const &value)
			: name(name), value(value) { }
		};

		/**
		 * User-defined command executed as response to a 'RING' URC
		 */
		struct Ring : Operation
		{
			Command const _command;
			Command command() const override { return _command; };

			Ring(Command const &command) : _command(command) { }
		};

		Constructible<Check_ready>       _check_ready       { };
		Constructible<Disable_echo>      _disable_echo      { };
		Constructible<Ring>              _ring              { };
		Constructible<Set_pin>           _set_pin           { };
		Constructible<Request_pin_state> _request_pin_state { };
		Constructible<Request_pin_count> _request_pin_count { };
		Constructible<Power_down>        _power_down        { };
		Constructible<Reboot>            _reboot            { };
		Constructible<Query_qcfg>        _query_qcfg        { };
		Constructible<Assign_qcfg>       _assign_qcfg       { };
		Constructible<Hang_up>           _hang_up           { };
		Constructible<Request_call_list> _request_call_list { };
		Constructible<Initiate_call>     _initiate_call     { };
		Constructible<Accept_call>       _accept_call       { };

		template <typename FN, typename LAST>
		void _apply_to(FN const &fn, LAST &last) { fn(last); }

		template <typename FN, typename HEAD, typename... TAIL>
		void _apply_to(FN const &fn, HEAD &head, TAIL &&... tail)
		{
			if (!fn(head))
				_apply_to(fn, tail...);
		}

		template <typename FN>
		void _apply_to_operations(FN const &fn)
		{
			_apply_to(fn, _check_ready,
			              _disable_echo,
			              _ring,
			              _set_pin,
			              _request_pin_state,
			              _request_pin_count,
			              _power_down,
			              _reboot,
			              _query_qcfg,
			              _assign_qcfg,
			              _hang_up,
			              _request_call_list,
			              _initiate_call,
			              _accept_call);
		}

		void _cancel_all_operations()
		{
			_apply_to_operations([&] (auto &op) {
				op.destruct();
				return false;
			});
		}

		void _log_pending_operations()
		{
			unsigned count = 0;

			_apply_to_operations([&] (auto &op) {
				if (op.constructed()) {
					if (count++ == 0)
						log("pending AT commands:");

					log("  command: ", op->command(), " state=", (int)op->state);
				}
				return false;
			});

			if (count == 0)
				log("no pending AT commands");
		}

		bool _any_operation_in_flight()
		{
			bool result = false;

			_apply_to_operations([&] (auto &op) {

				if (op.constructed() && op->state == Operation::State::SUBMITTED) {
					result = true;
					return true;
				}
				return false;
			});
			return result;
		}

		void _submit_one_pending_operation(Command_channel &command_channel)
		{
			auto try_submit = [&] (auto &op)
			{
				if (op.constructed() && op->state == Operation::State::PENDING) {
					command_channel.send_command_to_modem(op->command());
					op->state = Operation::State::SUBMITTED;
					return true;
				}
				return false;
			};

			_apply_to_operations(try_submit);
		}

		void _complete_operation()
		{
			if (!_status.ok)
				return;

			auto try_complete = [&] (auto &op)
			{
				if (op.constructed() && op->state == Operation::State::SUBMITTED) {
					op.destruct();
					return true;
				}
				return false;
			};

			_apply_to_operations(try_complete);
		}

		Pin      _current_pin   { };
		Power    _current_power { };
		unsigned _current_ring_count { };

		struct Outbound_info
		{
			using Call = Status::Current_call;

			Call     call;
			unsigned no_carrier_count;

			void gen_state_attr(Xml_generator &xml) const
			{
				if      (call.active())   xml.attribute("state", "active");
				else if (call.alerting()) xml.attribute("state", "alerting");
				else if (call.outbound()) xml.attribute("state", "outbound");
			}
		};

		Constructible<Outbound_info> _outbound_info { };

		bool _outbound(Number const &number) const
		{
			return _outbound_info.constructed()
			   && (_outbound_info->call.number == number);
		}

		bool _rejected(Number const &number) const
		{
			return _outbound_info.constructed()
			    && (_outbound_info->call.number == number)
			    && (_outbound_info->no_carrier_count != _status.no_carrier_count);
		}

		bool _ready_for_telephony() const
		{
			if (!_status.cpin.constructed())
				return false;

			return (_status.cpin->value == "READY");
		}

		void _apply_pin(Pin const &pin)
		{
			auto failed = [&] (auto const &request) {
				return request.constructed()
				    && (request->state == Operation::State::SUBMITTED)
				    && _status.cme_error.constructed(); };

			if (failed(_request_pin_state))
				_request_pin_state.destruct();

			if (failed(_set_pin)) {
				_set_pin.destruct();
				if (!_request_pin_count.constructed())
					_request_pin_count.construct();
				return;
			}

			/* pin status not yet known */
			if (!_status.cpin.constructed()) {
				if (!_request_pin_state.constructed())
					_request_pin_state.construct();
				return;
			}

			/* modem does not ask for a SIM PIN */
			if (_status.cpin->value != "SIM PIN")
				return;

			if (pin.length() <= 1)
				return;

			if (pin == _current_pin)
				return;

			_current_pin = pin;
			_set_pin.construct(pin);
		}

		void _keep_call_list_up_to_date()
		{
			if (!_ready_for_telephony())
				return;

			if (_status.clcc_up_to_date) {
				_request_call_list.destruct();
				return;
			}

			if (!_request_call_list.constructed())
				_request_call_list.construct();
		}

		bool _call_has_state(Xml_node const &call, char const *expected)
		{
			return (call.attribute_value("state", String<32>()) == expected);
		}

		void _apply_hang_up(Xml_node const &config)
		{
			/* hang up in progress */
			if (_hang_up.constructed())
				return;

			auto call_node_rejected = [&]
			{
				bool rejected = false;
				config.with_optional_sub_node("call", [&] (Xml_node const &call) {
					rejected = _call_has_state(call, "rejected"); });
				return rejected;
			};

			/* an initiated call got cancelled */
			if (_outbound_info.constructed()) {

				bool const call_node_valished = !config.has_sub_node("call");

				if (call_node_valished || call_node_rejected()) {
					_outbound_info.destruct();
					_hang_up.construct();
				}
			}

			/* reject incoming or active call */
			if (_status.current_call.constructed() && call_node_rejected())
				if (_status.current_call->incoming() || _status.current_call->active())
					_hang_up.construct();
		}

		void _apply_call(Xml_node const &call)
		{
			if (!_ready_for_telephony())
				return;

			/* initiate or accept calls only with up-to-date current-call info */
			if (!_status.clcc_up_to_date)
				return;

			Number const number   = call.attribute_value("number", Number());
			bool   const accepted = !call.has_attribute("state")
			                     || _call_has_state(call, "accepted");

			/* voice call present in call list */
			if (_status.current_call.constructed()) {

				/* keep note once an outbound call is featured in call list */
				if (_outbound_info.constructed())
					_outbound_info->call = *_status.current_call;

				/* accept incoming call */
				if (_status.current_call->incoming() && accepted)
					if (!_accept_call.constructed())
						_accept_call.construct();
			}

			/* number in config changed, issue new call */
			if (_outbound_info.constructed())
				if (_outbound_info->call.number != number)
					_outbound_info.destruct();

			bool const idle = !_status.current_call.constructed();

			/* issue call operation */
			if (idle && accepted && !_rejected(number) && !_outbound(number)) {

				if (!_initiate_call.constructed()) {
					_initiate_call.construct(number);

					Outbound_info const info {
						.call = { .number = number,
						          .stat   = Outbound_info::Call::Stat::DIALING },
						.no_carrier_count = _status.no_carrier_count,
					};
					_outbound_info.construct(info);
				}
			}
		}

		void _apply_power(Power const &power)
		{
			if (power == _current_power)
				return;

			if (power == "off") {
				if (!_power_down.constructed()) {
					_power_down.construct();
					_current_power = power;
				}
			}
		}

		void _apply_ring(Xml_node const &ring)
		{
			if (_current_ring_count != _status.ring_count) {
				_current_ring_count = _status.ring_count;
				_ring.construct(ring.decoded_content<Command>());
			}
		}

		void _apply_qcfg(Qcfg const &qcfg)
		{
			auto failed = [&] (auto const &request) {
				return request.constructed()
				    && (request->state == Operation::State::SUBMITTED)
				    && _status.error; };

			if (failed(_query_qcfg))
				_query_qcfg.destruct();

			/* query current modem configuration */
			if (!_query_qcfg.constructed())
				qcfg.with_any_unknown_entry([&] (Qcfg::Entry const &e) {
					_query_qcfg.construct(e.name); });

			/* assign mismatching configuration values */
			if (!_assign_qcfg.constructed())
				qcfg.with_any_mismatching_entry([&] (Qcfg::Entry const &e) {
					_assign_qcfg.construct(e.name, e.value); });

			/* reboot modem to ensure that changed settings become effective */
			if (!_reboot.constructed() && qcfg.reboot_needed()) {
				_cancel_all_operations();
				_reboot.construct();
				_current_pin = { };
			}
		}

		void _apply_config(Xml_node const &config, Qcfg const &qcfg)
		{
			/* don't issue AT commands during modem reboot */
			if (_reboot.constructed() || !_status.rdy)
				return;

			/* disable echo to avoid mixing up URC content with commands */
			_disable_echo.conditional(!_status.echo_disabled && _status.at_ok);

			if (!_status.echo_disabled)
				return;

			_apply_qcfg(qcfg);

			_apply_pin(config.attribute_value("pin", Pin()));

			_apply_power(config.attribute_value("power", Power()));

			_apply_hang_up(config);

			_keep_call_list_up_to_date();

			config.with_optional_sub_node("call", [&] (Xml_node const &call) {
				_apply_call(call); });

			config.with_optional_sub_node("ring", [&] (Xml_node const &ring) {
				_apply_ring(ring); });
		}

	public:

		Control(Status const &status) : _status(status) { }

		struct Verbose { bool value; };

		void apply_config(Xml_node const &config, Qcfg const &qcfg,
		                  Command_channel &command_channel,
		                  Verbose const verbose)
		{
			_complete_operation();

			/* issue 'AT' command until we get the response 'OK' */
			_check_ready.conditional(!_status.at_ok);

			_apply_config(config, qcfg);

			if (!_any_operation_in_flight())
				_submit_one_pending_operation(command_channel);

			if (verbose.value)
				_log_pending_operations();
		}

		void cancel_command()
		{
			_apply_to_operations([&] (auto &op) {
				if (op.constructed() && (op->state == Operation::State::SUBMITTED))
					op.destruct();
				return false;
			});
		}

		unsigned command_timeout_ms() const
		{
			auto currently_submitted = [&] (auto const &request) {
				return request.constructed()
				    && (request->state == Operation::State::SUBMITTED); };

			if (currently_submitted(_hang_up))
				return 90*1000; /* according to the Quectel documentation */

			return 600;
		}

		bool outbound() const { return _outbound_info.constructed(); }

		void gen_outbound_call(Xml_generator &xml) const
		{
			if (!outbound())
				return;

			xml.node("call", [&] {
				xml.attribute("number", _outbound_info->call.number);

				if (_rejected(_outbound_info->call.number)) {
					xml.attribute("state", "rejected");
					return;
				}

				_outbound_info->gen_state_attr(xml);
			});
		}
};

#endif /* _AT_PROTOCOL__CONTROL_H_ */
