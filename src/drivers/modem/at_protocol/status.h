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

#ifndef _AT_PROTOCOL__STATUS_H_
#define _AT_PROTOCOL__STATUS_H_

/* local includes */
#include <at_protocol/types.h>
#include <at_protocol/line.h>
#include <at_protocol/qcfg.h>

namespace At_protocol { struct Status; }


class At_protocol::Status : Noncopyable
{
	public:

		struct Changed : Interface
		{
			virtual void at_protocol_status_changed() = 0;
		};

		struct Version
		{
			unsigned value;
		};

	private:

		/**
		 * Attribute that may be unknown
		 */
		template <typename T>
		struct Optional : Constructible<T> { using Type = T; };

		Version _version { };  /* used for tracking state changes */

		void _status_changed() { _version.value++; }

		Qcfg &_qcfg;

		bool const &_verbose;

		template <typename T>
		void _apply_attr_value(T &optional, Line const &line)
		{
			using Attr = typename T::Type;
			line.with_value(Attr::prefix(), [&] (auto const &value) {
				_status_changed();
				optional.construct(Attr { value });
			});
		}

		void _apply_boolean(Line const &line, char const *string, bool &var, bool value)
		{
			if (line == string) {
				_status_changed();
				var = value;
			}
		}

		void _disable_boolean(Line const &line, char const *string, bool &var)
		{
			_apply_boolean(line, string, var, false);
		}

		void _enable_boolean(Line const &line, char const *string, bool &var)
		{
			_apply_boolean(line, string, var, true);
		}

		void _increment(Line const &line, char const *string, unsigned &var)
		{
			if (line == string) {
				_status_changed();
				var++;
			}
		}

		void _clcc_out_of_date()
		{
			clcc_up_to_date = false;
			current_call.destruct();
		}

		void _reset_command()
		{
			_pending_command.destruct();
			ok    = false;
			error = false;
			cme_error.destruct();
			_status_changed();
		}

		Constructible<Command> _pending_command { };

	public:

		bool at_ok           = false;
		bool echo_disabled   = false;
		bool powered_down    = false;
		bool ok              = false;
		bool rdy             = true;  /* modem may be RDY from previous boot */
		bool error           = false;
		bool clcc_up_to_date = false;

		unsigned ring_count               = 0;
		unsigned incorrect_password_count = 0;
		unsigned no_carrier_count         = 0;
		unsigned busy_count               = 0;

		struct Sim_pin_count
		{
			static char const *prefix() { return "+QPINC: \"SC\","; }
			String<16> value;
		};

		struct Cpin
		{
			static char const *prefix() { return "+CPIN: "; }
			String<16> value;
		};

		struct Cme_error
		{
			static char const *prefix() { return "+CME ERROR: "; }
			String<16> value;
		};

		/*
		 * Capture response of CLCC command
		 */
		struct Current_call
		{
			Number number;

			enum class Stat { UNSUPPORTED, ACTIVE, DIALING, INCOMING, ALERTING };

			Stat stat;

			template <typename FN>
			static void with_match(Line const &line, FN const &fn)
			{
				using Value = String<100>;

				auto stat_from_value = [] (Value const &v)
				{
					Value const field = comma_separated_element(2, v);

					if (field == "0") return Stat::ACTIVE;
					if (field == "2") return Stat::DIALING;
					if (field == "3") return Stat::ALERTING;
					if (field == "4") return Stat::INCOMING;

					return Stat::UNSUPPORTED;
				};

				line.with_value("+CLCC: ", [&] (Value const &value) {

					bool const voice = (comma_separated_element(3, value) == "0");

					Stat   const stat   = stat_from_value(value);
					Number const number = comma_separated_element(5, value);

					if (voice && stat != Stat::UNSUPPORTED)
						fn( Current_call { .number = number, .stat = stat } );
				});
			}

			bool active()   const { return stat == Stat::ACTIVE;   }
			bool outbound() const { return stat == Stat::DIALING;  }
			bool alerting() const { return stat == Stat::ALERTING; }
			bool incoming() const { return stat == Stat::INCOMING; }
		};

		Optional<Cpin>          cpin          { };
		Optional<Cme_error>     cme_error     { };
		Optional<Sim_pin_count> sim_pin_count { };

		Constructible<Current_call> current_call { };

		template <typename FN>
		void with_pending_command(FN const &fn) const
		{
			if (_pending_command.constructed())
				fn(*_pending_command);
		}

		void _apply_qcfg_response(Line const &line)
		{
			/* handle response to QCFG query */
			line.with_value("+QCFG: ", [&] (String<100> const &response) {

				Qcfg::Name const name = comma_separated_element(0, response);

				/*
				 * The attribute name appears in quotes. Retain all remaining
				 * comma-separated values in one string. Skip the attribute
				 * name length (value includes null termination), the quote
				 * characters and the comma after the attribute name.
				 */
				size_t const skip = name.length() + 2;
				if (response.length() < skip)
					return;

				Qcfg::Value const value(Cstring(response.string() + skip));

				_qcfg.with_entry(name, [&] (Qcfg::Entry &entry) {
					entry.state = (entry.value == value)
					            ? Qcfg::Entry::State::CONFIRMED
					            : Qcfg::Entry::State::MISMATCH;
					_status_changed();
				});
			});

			/* handle response to QCFG assignment */
			if (line == "OK") {

				with_pending_command([&] (Command const &command) {

					if (!starts_with(command, AT_QCFG_PREFIX))
						return;

					using Attr = String<100>;
					Attr const attr(Cstring(command.string() + strlen(AT_QCFG_PREFIX)));

					/* a QCFG assigment features at least one comma */
					Qcfg::Value const value = comma_separated_element(1, attr);
					if (value.length() <= 1)
						return;

					Qcfg::Name const name = comma_separated_element(0, attr);
					_qcfg.with_entry(name, [&] (Qcfg::Entry &e) {
						e.state = Qcfg::Entry::State::MODIFIED;
						_status_changed();
					});
				});
			}
		}

		void apply_line(Line const &line)
		{
			if (_verbose)
				log("modem: '", line, "'");

			/* whenever the modem sends anything, it cannot be powered down */
			powered_down = false;

			_apply_attr_value(cpin,          line);
			_apply_attr_value(cme_error,     line);
			_apply_attr_value(sim_pin_count, line);

			_apply_qcfg_response(line);

			Current_call::with_match(line, [&] (Current_call const &match) {
				current_call.construct(match);
				_status_changed();
			});

			_enable_boolean(line, "POWERED DOWN", powered_down);
			_enable_boolean(line, "OK",           ok);
			_enable_boolean(line, "ERROR",        error);
			_enable_boolean(line, "RDY",          rdy);

			_increment(line, "RING",           ring_count);
			_increment(line, "NO CARRIER",     no_carrier_count);
			_increment(line, "+CME ERROR: 10", busy_count);
			_increment(line, "+CME ERROR: 14", busy_count);
			_increment(line, "ERROR",          busy_count);

			/* discard outdated pin info */
			if (line == "+CME ERROR: 16")
				cpin.destruct();

			/* discard last known call list */
			if ((line == "NO CARRIER") || (line == "RING"))
				_clcc_out_of_date();

			/* gather result of 'Check_ready' and 'Disable_echo' */
			with_pending_command([&] (Command const &command) {
				if (command == AT_CHECK_READY)
					_enable_boolean(line, "OK", at_ok);
				if (command == AT_DISABLE_ECHO)
					_enable_boolean(line, "OK", echo_disabled);
			});

			/*
			 * The 'AT+CLCC' happens to not always respond with 'OK'.
			 * Accept URC with matching idx 2 as valid response.
			 */
			if (current_call.constructed())
				clcc_up_to_date = true;

			if (line == "OK") {

				with_pending_command([&] (Command const &command) {

					/* reboot confirmed */
					if (command == AT_REBOOT) {
						at_ok         = false;
						echo_disabled = false;
						rdy           = false;

						cpin         .destruct();
						cme_error    .destruct();
						sim_pin_count.destruct();

						_clcc_out_of_date();
						_qcfg.invalidate_after_reboot();
					}

					/* detect end of empty call list */
					if (command == AT_REQUEST_CALL_LIST)
						clcc_up_to_date = true;

					/* certain commands have a side effect on the call list */
					bool const clcc_invalidated = starts_with(command, "ATD")
					                          || (command == AT_HANG_UP)
					                          || (command == AT_ACCEPT_CALL);
					if (clcc_invalidated)
						_clcc_out_of_date();
				});
			}

			/* detect completion/failure of current command */
			if (line == "OK" || line == "ERROR" || line.starts_with("+CME ERROR:"))
				_pending_command.destruct();
		}

		void command_submitted(Command const &command)
		{
			_reset_command();
			_pending_command.construct(command);

			if (command == AT_REQUEST_CALL_LIST)
				_clcc_out_of_date();
		}

		void command_canceled() { _reset_command(); }

		Version version() const { return _version; }

		Status(Qcfg &qcfg, bool const &verbose) : _qcfg(qcfg), _verbose(verbose) { }
};

#endif /* _AT_PROTOCOL__STATUS_H_ */
