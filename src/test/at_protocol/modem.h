/*
 * \brief  Modem role for AT protocol test
 * \author Norman Feske
 * \date   2022-06-15
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _MODEM_H_
#define _MODEM_H_

/* Genode includes */
#include <timer_session/connection.h>
#include <terminal_session/connection.h>

/* local includes */
#include <at_protocol/driver.h>

namespace Test {

	using namespace At_protocol;

	struct Modem;
}


struct Test::Modem : Noncopyable
{
	Env &_env;

	Terminal::Connection _terminal { _env };

	using Read_buffer = At_protocol::Read_buffer<256>;

	Read_buffer _read_buffer { };

	Timer::Connection _sleep_timer   { _env };
	Timer::Connection _timeout_timer { _env };

	Signal_handler<Modem> _timeout_handler {
		_env.ep(), *this, &Modem::_handle_timeout };

	Signal_handler<Modem> _read_from_driver_handler {
		_env.ep(), *this, &Modem::_handle_read_from_driver };

	struct Delay { unsigned long ms; };

	/* counter used to drop 'AT' commands a few times (modem not ready yet) */
	unsigned _check_ready_count = 0;

	bool _pin_valid              = false;
	bool _ringing                = false;
	bool _call_established       = false;
	bool _initiated_invalid_call = false;
	bool _initiated_valid_call   = false;
	bool _audible_ring           = false;

	void _handle_timeout()
	{
		if (_initiated_invalid_call) {
			_modem_sends("NO CARRIER\r\n");
			_initiated_invalid_call = false;
		}
		if (_initiated_valid_call) {
			_initiated_valid_call = false;
			_call_established     = true;
		}
		if (_ringing) {
			_modem_sends("RING\r\n");
			_schedule_timeout_in_one_second();
		}

		_handle_read_from_driver();
	}

	void _schedule_timeout_in_one_second()
	{
		_timeout_timer.trigger_once(1000*1000);
	}

	void _modem_sends_delayed(char const *s, Delay delay)
	{
		_sleep_timer.msleep(delay.ms);
		_terminal.write(s, strlen(s));
	}

	void _modem_sends(char const *s)
	{
		/* discourage merging of data transfers */
		_modem_sends_delayed(s, Delay { .ms = 100 });
	}

	void _handle_command(Line const &line)
	{
		log("got command: ", line);

		/*
		 * Magic commands issued by the driver side for driving the test.
		 */

		if (line == "remote: ring") {
			_ringing = true;
			_schedule_timeout_in_one_second();
		}

		if (line == "remote: hangup") {
			_ringing = false;
			_modem_sends("NO CARRIER\r\n");
			_call_established = false;
		}

		/*
		 * AT protocol commands
		 */

		/* alive */
		if (line == "AT") {
			/* simulate the modem booting up, not yet responding to commands */
			_check_ready_count++;
			if (_check_ready_count > 3)
				_modem_sends("OK\r\n");
		}

		/* disable echo */
		if (line == "ATE0") {
			_modem_sends("OK\r\n");
		}

		/* set wrong pin */
		if (line == "AT+CPIN=4321") {
			_modem_sends("+CME ERROR: 16\r\n");
		}

		/* request pin state */
		if (line == "AT+CPIN?") {
			if (_pin_valid) {
				_modem_sends("+CPIN: READY\r\n");
				_modem_sends("OK\r\n");
			} else {
				_modem_sends("+CPIN: SIM PIN\r\n");
				_modem_sends("OK\r\n");
			}
		}

		/* request pin counter */
		if (line == "AT+QPINC?") {
			_modem_sends("+QPINC: \"SC\",2,10\r\n");
			_modem_sends("+QPINC: \"P2\",3,10\r\n");
			_modem_sends("OK\r\n");
		}

		/* set correct pin */
		if (line == "AT+CPIN=1234") {
			_modem_sends("OK\r\n");
			_modem_sends("+CPIN: READY\r\n");
			_modem_sends("+QUSIM: 1\r\n");       /* deliver bunch of URCs */
			_modem_sends("+QIND: SMS DONE\r\n");
			_modem_sends("+QIND: PB DONE\r\n");
		}

		/* list callers */
		if (line == "AT+CLCC") {
			if (_initiated_valid_call) {
				_modem_sends("+CLCC: 1,1,0,1,0,\"\",128\r\n");
				_modem_sends("+CLCC: 2,0,3,0,0,\"+49123123123\",129\r\n");
				_modem_sends("OK\r\n");
			}
			else if (_initiated_invalid_call) {
				_modem_sends("+CLCC: 1,1,0,1,0,\"\",128\r\n");
				_modem_sends("+CLCC: 2,0,3,0,0,\"03519999\",129\r\n");
				_modem_sends("OK\r\n");
			}
			else if (!_ringing && !_call_established) {
				_modem_sends("+CLCC: 1,1,0,1,0,\"\",128\r\n");
				_modem_sends("OK\r\n");
			}
			else if (_ringing) {
				_modem_sends("+CLCC: 1,1,0,1,0,\"\",128\r\n");
				_modem_sends("+CLCC: 2,1,4,0,0,\"+49123123123\",145\r\n");
				_modem_sends("OK\r\n");
			}
			else if (_call_established) {
				_modem_sends("+CLCC: 1,1,0,1,0,\"\",128\r\n");
				_modem_sends("+CLCC: 2,1,0,0,0,\"+49123123123\",145\r\n");
				_modem_sends("OK\r\n");
			}
		}

		/* initiate call to invalid number or a callee denying the call */
		if (line == "ATD03519999;") {
			_modem_sends("OK\r\n");
			_initiated_invalid_call = true;
			_schedule_timeout_in_one_second();
		}

		/* initiate call to valid number */
		if (line == "ATD+49123123123;") {
			_modem_sends("OK\r\n");
			_initiated_valid_call = true;
			_schedule_timeout_in_one_second();
		}

		/* accept incoming call */
		if (line == "ATA") {
			if (_ringing) {
				_modem_sends("OK\r\n");
				_call_established = true;
				_ringing = false;
			} else {
				_modem_sends("NO CARRIER\r\n");
			}
		}

		/* hang up */
		if (line == "ATH") {
			_modem_sends("OK\r\n");
			_call_established = false;
		}

		/* power down */
		if (line == "AT+QPOWD") {
			_modem_sends_delayed("POWERED DOWN\r\n", Delay { .ms = 1000 });

			/*
			 * This is the last step of the test.
			 *
			 * Report success only if at least one ring command was issued
			 * during test. There is no way to check this condition at the
			 * driver side.
			 */
			if (_audible_ring)
				log("Test done.");
		}

		/* ring command */
		if (line == "AT+QLDTMF=5,\"4,3,6,#,D,3\",1") {
			log("playing audible tone sequence...");
			_audible_ring = true;
			_modem_sends("OK\r\n");
		}
	}

	void _handle_read_from_driver()
	{
		/*
		 * Divert 'Read_buffer' as utility for obtaining commands from the
		 * driver.
		 */
		struct Terminal_channel : Response_channel
		{
			Terminal::Connection &_terminal;

			size_t read_from_modem(void *buf, size_t buf_len) override
			{
				return _terminal.read(buf, buf_len);
			}

			Terminal_channel(Terminal::Connection &terminal) : _terminal(terminal) { }

		} terminal_channel { _terminal };

		for (unsigned i = 0; ; i++) {

			if (_read_buffer.fill(terminal_channel) == Read_buffer::Fill_result::DONE)
				break;

			_read_buffer.consume_lines([&] (Line const &line) {
				_handle_command(line); });
		}
	}

	Modem(Env &env) : _env(env)
	{
		_timeout_timer.sigh(_timeout_handler);

		_terminal.read_avail_sigh(_read_from_driver_handler);
		_handle_read_from_driver();
	}
};

#endif /* _MODEM_H_ */
