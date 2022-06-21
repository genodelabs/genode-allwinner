/*
 * \brief  Helper for parsing lines received from the modem
 * \author Norman Feske
 * \date   2022-06-15
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _AT_PROTOCOL__LINE_H_
#define _AT_PROTOCOL__LINE_H_

/* Genode includes */
#include <util/string.h>

/* AT-protocol includes */
#include <at_protocol/types.h>

namespace At_protocol { struct Line; }


class At_protocol::Line : Noncopyable
{
	private:

		Byte_range_ptr const _ptr;

	public:

		Line(Byte_range_ptr ptr) : _ptr(ptr) { };

		bool starts_with(char const *s) const
		{
			size_t const len = strlen(s);

			return (_ptr.num_bytes >= len)
			    && (strcmp(s, _ptr.start, len) == 0);
		}

		bool operator == (char const *s) const
		{
			size_t const len = strlen(s);

			return (_ptr.num_bytes == len)
			    && (strcmp(s, _ptr.start, len) == 0);
		}

		void print(Output &out) const
		{
			Genode::print(out, Cstring(_ptr.start, _ptr.num_bytes));
		}

		template <typename FN>
		void with_value(char const *prefix, FN const &fn) const
		{
			if (!starts_with(prefix))
				return;

			size_t const prefix_len = strlen(prefix);

			fn(Cstring(_ptr.start     + prefix_len,
			           _ptr.num_bytes - prefix_len));
		}
};

#endif /* _AT_PROTOCOL__LINE_H_ */
