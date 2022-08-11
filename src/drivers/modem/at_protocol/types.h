/*
 * \brief  Types used by the AT-protocol handler
 * \author Norman Feske
 * \date   2022-06-15
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _AT_PROTOCOL__TYPES_H_
#define _AT_PROTOCOL__TYPES_H_

#include <util/xml_generator.h>

namespace At_protocol {

	using namespace Genode;

	using Command = String<256>;
	using Number  = String<50>;   /* phone number */

	static constexpr char const * AT_CHECK_READY       = "AT";
	static constexpr char const * AT_REQUEST_CALL_LIST = "AT+CLCC";
	static constexpr char const * AT_HANG_UP           = "ATH";
	static constexpr char const * AT_ACCEPT_CALL       = "ATA";
	static constexpr char const * AT_POWER_DOWN        = "AT+QPOWD";
	static constexpr char const * AT_DISABLE_ECHO      = "ATE0";
	static constexpr char const * AT_QCFG_PREFIX       = "AT+QCFG=";
	static constexpr char const * AT_REBOOT            = "AT+CFUN=1,1";

	template <size_t N>
	static bool starts_with(String<N> const &, char const *prefix);

	template <size_t N>
	static String<N> comma_separated_element(unsigned, String<N> const &);

	struct Command_channel : Interface
	{
		virtual void send_command_to_modem(Command const &) = 0;
	};

	struct Response_channel : Interface
	{
		virtual size_t read_from_modem(void *buf, size_t buf_len) = 0;
	};
}


template <Genode::size_t N>
bool At_protocol::starts_with(String<N> const &string, char const *prefix)
{
	size_t const len = strlen(prefix);

	return (string.length() > len)
	    && (strcmp(prefix, string.string(), len) == 0);
};


template <Genode::size_t N>
static Genode::String<N> At_protocol::comma_separated_element(unsigned idx, String<N> const &s)
{
	using Token = Genode::Token<Scanner_policy_identifier_with_underline>;

	unsigned commas = 0;

	for (Token t(s.string(), s.length() - 1); t.type() != Token::END; t = t.next()) {

		if (t.type() == Token::SINGLECHAR) {
			if (t[0] == ',')
				commas++;
			continue;
		}

		if (idx != commas)
			continue;

		/* reached element 'idx' */

		if (t.type() == Token::NUMBER)
			return String<N>(Cstring(t.start(), t.len()));

		if (t.type() == Token::STRING)
			return String<N>(Cstring(t.start() + 1, t.len() - 2));
	}

	return String<N>();
}

#endif /* _AT_PROTOCOL__TYPES_H_ */
