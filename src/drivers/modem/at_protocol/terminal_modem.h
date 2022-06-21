/*
 * \brief  Command_channel and Response_channel based on a terminal connection
 * \author Norman Feske
 * \date   2022-06-24
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _AT_PROTOCOL__TERMINAL_MODEM_H_
#define _AT_PROTOCOL__TERMINAL_MODEM_H_

/* Genode includes */
#include <terminal_session/connection.h>

/* AT-protocol includes */
#include <at_protocol/types.h>

namespace At_protocol { struct Terminal_modem; }


struct At_protocol::Terminal_modem : Command_channel, Response_channel
{
	Terminal::Connection _terminal;

	Terminal_modem(Env &env) : _terminal(env) { }

	~Terminal_modem()
	{
		_terminal.read_avail_sigh(Signal_context_capability());
	}

	void sigh(Signal_context_capability sigh)
	{
		_terminal.read_avail_sigh(sigh);
	}

	/**
	 * Command_channel interface
	 */
	void send_command_to_modem(Command const &command) override
	{
		/* append carriage return and newline characters */
		Command const command_with_eol(command, "\r\n");

		size_t const len = strlen(command_with_eol.string());

		if (len == Command::capacity())
			warning("modem command got unexpectedly truncated\n");

		if (_terminal.write(command_with_eol.string(), len) != len)
			warning("modem command too large for terminal write\n");
	}

	/**
	 * Response_channel interface
	 */
	size_t read_from_modem(void *buf, size_t buf_len) override
	{
		return _terminal.read(buf, buf_len);
	}
};

#endif /* _AT_PROTOCOL__TERMINAL_MODEM_H_ */
