/*
 * \brief  Driver role for AT protocol test
 * \author Norman Feske
 * \date   2022-06-15
 *
 * Open issues:
 * - What happens if a modem command times out?
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _DRIVER_H_
#define _DRIVER_H_

/* Genode includes */
#include <base/env.h>

/* AT-protocol includes */
#include <at_protocol/terminal_modem.h>
#include <at_protocol/driver.h>

namespace Test { struct Driver; }


/**
 * Check the correct parsing of lines
 */
static void check_small_read_buffer()
{
	using namespace At_protocol;

	Read_buffer<8> read_buffer { };

	struct Channel : Response_channel
	{
		String<32> content { };

		size_t read_from_modem(void *buf, size_t buf_len) override
		{
			size_t const len = strlen(content.string());

			if (len > buf_len) {
				error("unexpectedly small buf_len argument for 'read_from_modem'");
				throw 1;
			}

			memcpy(buf, content.string(), len);
			return len;
		}
	};

	using Expected = String<16>;
	Expected const expected_lines[] { "abcd", "efghijk", "x", "ok" };

	unsigned const num_lines = sizeof(expected_lines)/sizeof(expected_lines[0]);

	unsigned curr_line = 0;

	auto match_expected = [&] (Line const &line)
	{
		if (curr_line >= num_lines) {
			error("got more lines than expected");
			throw 2;
		}

		Expected const expected = expected_lines[curr_line++];
		if (line == expected.string())
			return;

		error("line '", line, "' does not match expected line '", expected, "'");
		throw 3;
	};

	Channel channel { };

	auto modem_sends = [&] (char const *content)
	{
		channel.content = content;
		read_buffer.fill(channel);
		read_buffer.consume_lines([&] (Line const &line) {
			log("got line: ", line);
			match_expected(line);
		});
	};

	/* lines split accross multiple transfers */
	modem_sends("abcd\r\ne");
	modem_sends("fghijk\r\nx");
	modem_sends("\r\n");

	/* overly long line, must be discarded */
	modem_sends("abcdefghijk");

	/* detect subsequent line following the discarded data */
	modem_sends("ok\r\n");

	log("Test check_small_read_buffer passed.");
};


struct Test::Driver : Noncopyable
{
	Env &_env;

	At_protocol::Terminal_modem _modem { _env };

	Signal_handler<Driver> _read_from_modem_handler {
		_env.ep(), *this, &Driver::_handle_read_from_modem };

	using Protocol_driver = At_protocol::Driver<256>;

	Protocol_driver _protocol_driver { };

	/* asserted modem configuration */
	At_protocol::Qcfg::Entry _qcfg_usbnet_ecm {
		_protocol_driver.qcfg, "usbnet", "1" };
	At_protocol::Qcfg::Entry _qcfg_sms_ri {
		_protocol_driver.qcfg, "urc/ri/smsincoming", "pulse,120,1" };

	Timer::Connection _timer { _env };

	Signal_handler<Driver> _timer_handler {
		_env.ep(), *this, &Driver::_handle_timer };

	void _handle_timer()
	{
		/* cancel timed-out command */
		if (_at_response_outstanding()) {
			uint64_t const duration_ms = _timer.elapsed_ms() - _last_progress_ms;
			if (duration_ms > 600) {
				log("command timed out");
				_protocol_driver.cancel_command();
			}
		}

		_handle_read_from_modem();
	}

	uint64_t _last_progress_ms = 0;  /* for handling command timeouts */

	bool _at_response_outstanding() const
	{
		return _protocol_driver.response_outstanding();
	}

	void _schedule_timer_in_500ms()
	{
		log("schedule timer in 500 ms");
		_timer.trigger_once(500*1000);
	}

	/* test plan executed at the second stage */
	enum class Step {
		WRONG_PIN,
		INITIATE_DENIED_CALL,
		INITIATE_ACCEPTED_CALL,
		HANGUP,
		INCOMING_CALL,
		ACCEPT_INCOMING_CALL,
		REMOTE_HANGS_UP,
		POWERING_DOWN,
		POWERED_DOWN,
		DONE,
	};

	static const char *_step_name(Step step)
	{
		switch (step) {
		case Step::WRONG_PIN:              return "WRONG_PIN";
		case Step::INITIATE_DENIED_CALL:   return "INITIATE_DENIED_CALL";
		case Step::INITIATE_ACCEPTED_CALL: return "INITIATE_ACCEPTED_CALL";
		case Step::HANGUP:                 return "HANGUP";
		case Step::INCOMING_CALL:          return "INCOMING_CALL";
		case Step::ACCEPT_INCOMING_CALL:   return "ACCEPT_INCOMING_CALL";
		case Step::REMOTE_HANGS_UP:        return "REMOTE_HANGS_UP";
		case Step::POWERING_DOWN:          return "POWERING_DOWN";
		case Step::POWERED_DOWN:           return "POWERED_DOWN";
		case Step::DONE:                   return "DONE";
		};
		return "<unknown>";
	};

	Step _step = Step::WRONG_PIN;

	/*
	 * State machine for the second test stage
	 */
	void _apply_state_changes(Xml_node const &state)
	{
		auto apply_config = [&] (Xml_node const &config)
		{
			log("apply config: ", config);
			_protocol_driver.apply(config, _modem, _modem);
		};

		log("--- step ", _step_name(_step), " ---\nstate: ", state);

		using Value = String<128>;

		/* return true if current call with given number is in expected state */
		auto call_state = [&] (auto number, auto expected)
		{
			bool result = false;
			state.with_optional_sub_node("call", [&] (Xml_node const &call) {
				result = (call.attribute_value("number", Value()) == number)
				      && (call.attribute_value("state",  Value()) == expected); });
			return result;
		};

		switch (_step) {

		case Step::WRONG_PIN:

			apply_config(Xml_node("<config pin=\"4321\" />"));

			if (state.attribute_value("pin", Value()) != "required")
				return;

			if (state.attribute_value("pin_remaining_attempts", 0u) != 2)
				return;

			_step = Step::INITIATE_DENIED_CALL;
			return;

		case Step::INITIATE_DENIED_CALL:

			apply_config(Xml_node("<config pin=\"1234\">"
			                      "  <call number=\"03519999\"/>"
			                      "</config>"));

			if (state.attribute_value("pin", Value()) != "ok")
				return;

			if (!call_state("03519999", "rejected"))
				return;

			_step = Step::INITIATE_ACCEPTED_CALL;
			return;

		case Step::INITIATE_ACCEPTED_CALL:

			apply_config(Xml_node("<config pin=\"1234\">"
			                      "  <call number=\"+49123123123\"/>"
			                      "</config>"));

			if (!call_state("+49123123123", "active"))
				return;

			_step = Step::HANGUP;
			return;

		case Step::HANGUP:

			apply_config(Xml_node("<config pin=\"1234\">"
			                      "  <call number=\"+49123123123\" state=\"rejected\"/>"
			                      "</config>"));

			if (state.has_sub_node("call"))
				return;

			_protocol_driver.send_command_to_modem(_modem, "remote: ring");
			_step = Step::INCOMING_CALL;
			return;

		case Step::INCOMING_CALL:

			apply_config(Xml_node("<config pin=\"1234\">"
			                      "  <ring>AT+QLDTMF=5,\"4,3,6,#,D,3\",1</ring>"
			                      "</config>"));

			if (state.attribute_value("ring_count", 0u) < 3)
				return;

			if (!call_state("+49123123123", "incoming"))
				return;

			_step = Step::ACCEPT_INCOMING_CALL;
			return;

		case Step::ACCEPT_INCOMING_CALL:

			apply_config(Xml_node("<config pin=\"1234\">"
			                      "  <call number=\"+49123123123\"/>"
			                      "</config>"));

			if (!call_state("+49123123123", "active"))
				return;

			_protocol_driver.send_command_to_modem(_modem, "remote: hangup");
			_step = Step::REMOTE_HANGS_UP;
			return;

		case Step::REMOTE_HANGS_UP:

			apply_config(Xml_node("<config pin=\"1234\">"
			                      "  <call number=\"+49123123123\"/>"
			                      "</config>"));

			if (!call_state("+49123123123", "rejected"))
				return;

			_step = Step::POWERING_DOWN;
			return;

		case Step::POWERING_DOWN:

			apply_config(Xml_node("<config power=\"off\"/>"));

			if (!_protocol_driver.powering_down())
				return;

			_step = Step::POWERED_DOWN;
			return;

		case Step::POWERED_DOWN:

			apply_config(Xml_node("<config power=\"off\"/>"));

			if (!_protocol_driver.powered_down())
				return;

			_step = Step::DONE;
			log("Test done.");
			return;

		case Step::DONE:
			return;
		};
	}

	void _handle_read_from_modem()
	{
		/* express current AT status as XML report */
		char report_buffer[1024] { };
		Xml_generator xml(report_buffer, sizeof(report_buffer), "modem", [&] {
			_protocol_driver.generate_report(xml); });

		/* simulate behavior of modem-driver user based on the report */
		for (;;) {
			Status::Version orig_version = _protocol_driver.status.version();

			_apply_state_changes(Xml_node(report_buffer));

			if (orig_version.value == _protocol_driver.status.version().value) {
				break;
			} else {
				_last_progress_ms = _timer.elapsed_ms();
			}
		}

		bool const need_polling = !_protocol_driver.status.at_ok
		                        || _protocol_driver.outbound()
		                        || _protocol_driver.response_outstanding();

		if (_protocol_driver.outbound())
			_protocol_driver.invalidate_call_list();

		if (need_polling)
			_schedule_timer_in_500ms();
	}

	Driver(Env &env) : _env(env)
	{
		check_small_read_buffer();

		_protocol_driver.verbose = true;
		_modem.sigh(_read_from_modem_handler);
		_timer.sigh(_timer_handler);

		_handle_read_from_modem();
	}
};

#endif /* _DRIVER_H_ */
