/*
 * \brief  Buffer for data received from the modem
 * \author Norman Feske
 * \date   2022-06-15
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _AT_PROTOCOL__READ_BUFFER_H_
#define _AT_PROTOCOL__READ_BUFFER_H_

/* AT-protocol includes */
#include <at_protocol/types.h>
#include <at_protocol/line.h>

namespace At_protocol { template <size_t> struct Read_buffer; }


template <Genode::size_t MAX_LINE_LEN>
struct At_protocol::Read_buffer : Noncopyable
{
	char _buffer[2*MAX_LINE_LEN] { };

	static bool _end_of_line(char c)
	{
		return c == 13 /* newline */
		    || c == 10 /* carriage return */ ;
	}

	size_t payload_bytes = 0;

	enum class Fill_result { DATA, DONE };

	Fill_result fill(Response_channel &modem)
	{
		size_t const remaining_capacity = sizeof(_buffer) - payload_bytes;

		size_t const appended_bytes = modem.read_from_modem(_buffer + payload_bytes,
		                                                    remaining_capacity);
		if (appended_bytes == 0)
			return Fill_result::DONE;

		payload_bytes += appended_bytes;

		return Fill_result::DATA;
	}

	template <typename FN>
	void consume_lines(FN const &fn)
	{
		/**
		 * Return number of consecutive characters matching 'fn'
		 */
		auto scan = [] (char const *s, size_t max_len, auto fn)
		{
			unsigned count = 0;
			for ( ; count < max_len && fn(*s); count++)
				s++;

			return count;
		};

		auto scan_for_end_of_line = [&] (char const *s, size_t max_len)
		{
			return scan(s, max_len, [] (char c) { return !_end_of_line(c); });
		};

		auto end_of_line_chars = [&] (char const *s, size_t max_len)
		{
			return scan(s, max_len, [] (char c) { return _end_of_line(c); });
		};

		for (;;) {

			if (payload_bytes == 0)
				break;

			size_t const line_len  = scan_for_end_of_line(_buffer, payload_bytes);

			bool const line_complete = (line_len < payload_bytes);
			if (!line_complete) {
				if (payload_bytes > MAX_LINE_LEN) {
					error("incoming modem data exceeds maximum line length");
					payload_bytes = 0;
				}
				break;
			}

			fn(Line(Byte_range_ptr(_buffer, line_len)));

			size_t const eol_chars = end_of_line_chars(&_buffer[line_len],
			                                           payload_bytes - line_len);

			size_t const consumed_len = line_len + eol_chars;

			memmove(_buffer, _buffer + consumed_len, payload_bytes - consumed_len);

			payload_bytes -= consumed_len;
		}
	}

	Read_buffer() { }
};

#endif /* _AT_PROTOCOL__READ_BUFFER_H_ */
