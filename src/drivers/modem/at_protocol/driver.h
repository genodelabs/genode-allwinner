/*
 * \brief  AT-protocol driver
 * \author Norman Feske
 * \date   2022-06-15
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _AT_PROTOCOL_DRIVER_H_
#define _AT_PROTOCOL_DRIVER_H_

/* AT-protocol includes */
#include <at_protocol/read_buffer.h>
#include <at_protocol/status.h>
#include <at_protocol/control.h>
#include <at_protocol/qcfg.h>

namespace At_protocol { template <size_t> class Driver; }


template <Genode::size_t MAX_LINE_LEN>
class At_protocol::Driver : Noncopyable
{
	public:

		bool verbose = false;

		Qcfg qcfg { };

		Status status { qcfg, verbose };

	private:

		Read_buffer<MAX_LINE_LEN> _read_buffer { };

		Control _control { status };

		/**
		 * Filter used to capture the information about the most recent command
		 * in the 'At_protocol::Status'.
		 */
		struct Command_filter : Command_channel
		{
			Command_channel &_channel;
			Status          &_status;
			bool       const _verbose;

			Command_filter(Command_channel &channel, Status &status, bool verbose)
			: _channel(channel), _status(status), _verbose(verbose) { }

			/**
			 * Command_channel interface
			 */
			void send_command_to_modem(Command const &command) override
			{
				if (_verbose)
					log("submit AT command: ", command);

				_channel.send_command_to_modem(command);
				_status.command_submitted(command);
			}
		};

		Constructible<Status::Current_call> _current_call { };

		void _gen_current_call(Xml_generator &xml) const
		{
			if (!_current_call.constructed())
				return;

			Status::Current_call const call = *_current_call;

			using Stat = Status::Current_call::Stat;

			if (call.stat == Stat::UNSUPPORTED)
				return;

			auto state_name = [] (Stat stat)
			{
				switch (stat) {
				case Stat::ACTIVE:      return "active";
				case Stat::DIALING:     return "outbound";
				case Stat::INCOMING:    return "incoming";
				case Stat::ALERTING:    return "alerting";
				case Stat::UNSUPPORTED: break;
				}
				return "unsupported";
			};

			xml.node("call", [&] {
				xml.attribute("number", call.number);
				xml.attribute("state", state_name(call.stat));
			});
		}

		unsigned _command_count { };

	public:

		void apply(Xml_node const &config,
		           Command_channel  &command_channel,
		           Response_channel &response_channel)
		{
			unsigned const orig_no_carrier_count = status.no_carrier_count;

			for (;;) {

				using Fill_result = typename Read_buffer<MAX_LINE_LEN>::Fill_result;

				if (_read_buffer.fill(response_channel) == Fill_result::DONE)
					break;

				_read_buffer.consume_lines([&] (Line const &line) {
					status.apply_line(line); });
			}

			Command_filter command_filter { command_channel, status, verbose };

			_control.apply_config(config, qcfg, command_filter,
			                      Control::Verbose { verbose });

			/* invalidate current-call information on disconnect */
			if (orig_no_carrier_count != status.no_carrier_count)
				_current_call.destruct();

			/* import current-call information from updated call list */
			if (status.clcc_up_to_date) {
				_current_call.destruct();
				if (status.current_call.constructed())
					_current_call.construct(*status.current_call);
			}
		}

		void generate_report(Xml_generator &xml) const
		{
			if (status.ring_count > 0)
				xml.attribute("ring_count", status.ring_count);

			if (status.no_carrier_count > 0)
				xml.attribute("no_carrier_count", status.no_carrier_count);

			/* pin information */
			if (status.cpin.constructed()) {

				xml.attribute("sim", "yes");

				if (status.cpin->value == "READY") {
					xml.attribute("pin", "ok");
				}
				else if (status.cpin->value == "SIM PIN") {
					xml.attribute("pin", "required");
					if (status.sim_pin_count.constructed())
						xml.attribute("pin_remaining_attempts",
						              status.sim_pin_count->value);
				}
			}

			/*
			 * Report information about an initiated call that is not
			 * represented in 'Status::current_call'. This can happen on a
			 * freshly initiated call, or when an initiated call got rejected.
			 */
			if (_control.outbound())
				_control.gen_outbound_call(xml);
			else
				_gen_current_call(xml);
		}

		/**
		 * Issue command to the modem
		 *
		 * In contrast to a direct call of 'send_command_to_modem' at the
		 * command channel, this method registers the supplied command
		 * at the 'At_protocol::Status' and applies the verbosity handling.
		 *
		 * This method is public only for the use by test-at_protocol.
		 */
		void send_command_to_modem(Command_channel &channel, Command const &command)
		{
			Command_filter(channel, status, verbose).send_command_to_modem(command);
		}

		/**
		 * Return true if an outbound call is in progress
		 *
		 * In the phase after placing a call via the 'ATD' command until the
		 * receiver is picking up the call, we have to actively poll the call
		 * list to determing the state of the outbound call by periodically
		 * calling 'apply_config'.
		 */
		bool outbound() const { return _control.outbound(); }

		/**
		 * Force an update of the call list
		 *
		 * This method needs to be called when polling the call state during
		 * an outbound call.
		 */
		void invalidate_call_list() { status.clcc_up_to_date = false; }

		/**
		 * Cancel current command
		 *
		 * This method is the designated response to timeouts.
		 */
		void cancel_command()
		{
			status.with_pending_command([&] (Command const &command) {

				if (verbose)
					log("cancel AT command: ", command);

				_control.cancel_command();
				status.command_canceled();
			});

			/*
			 * The command was canceled but the modem may still be busy with
			 * processing it, disregarding new command characters at that point.
			 * We trigger the submission of an AT command (with the expected
			 * "OK" as response) to attain a consistent protocol state where
			 * the next command submitted is recognised by the modem.
			 */
			status.at_ok = false;
		}

		/**
		 * Return timeout of currently executed command in miliseconds
		 */
		unsigned command_timeout_ms() const
		{
			return _control.command_timeout_ms();
		}

		/**
		 * Return true if the modem's response to an AT command is expected
		 */
		bool response_outstanding() const
		{
			bool result = false;

			status.with_pending_command([&] (Command const &) {
				result = true; });

			return result;
		}

		/**
		 * Return true if the shutdown procedure of the modem is in progress
		 */
		bool powering_down() const
		{
			bool result = false;

			status.with_pending_command([&] (Command const &command) {
				result = (command == AT_POWER_DOWN) && !status.powered_down; });

			return result;
		}

		/**
		 * Return true if the shutdown procedure of the modem is complete
		 *
		 * After the return value of this function becomes true, it is
		 * recommended to wait another second before revoking the modem's
		 * power supply.
		 */
		bool powered_down() const { return status.powered_down; }

		/**
		 * Return indicator that command processing must be deferred
		 *
		 * The pin state is not available while booting the SIM card. Whenever
		 * in increasing count is observed, it is best to wait a bit.
		 *
		 * Another case is the queriying of QCFG attributes while the modem
		 * is booting. E.g., when requesting "usbnet" too early, the modem
		 * responds with a plain "ERROR". When queried at a later time, the
		 * command succeeds.
		 */
		unsigned busy_count() const
		{
			return status.busy_count;
		}
};

#endif /* _AT_PROTOCOL_DRIVER_H_ */
