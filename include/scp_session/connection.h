/*
 * \brief  Connection to system-control processor (SCP)
 * \author Norman Feske
 * \date   2022-05-11
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__SCP_SESSION__CONNECTION_H_
#define _INCLUDE__SCP_SESSION__CONNECTION_H_

#include <scp_session/scp_session.h>
#include <base/connection.h>
#include <base/attached_dataspace.h>

namespace Scp { class Connection; }


class Scp::Connection : Genode::Connection<Session>, Rpc_client<Session>
{
	public:

		enum { RAM_QUOTA = 20*1024UL };

	private:

		Entrypoint &_ep;

		Attached_dataspace _ds;

		Io_signal_handler<Connection> _response_io_handler;

		unsigned _response_count = 0;

		void _handle_response() { _response_count++; }

	public:

		Connection(Env &env, char const *label = "")
		:
			Genode::Connection<Scp::Session>(
				env, session(env.parent(),
				             "ram_quota=%u, cap_quota=%u, label=\"%s\"",
				             RAM_QUOTA, CAP_QUOTA, label)),
			Rpc_client<Session>(cap()),
			_ep(env.ep()),
			_ds(env.rm(), call<Rpc_dataspace>()),
			_response_io_handler(_ep, *this, &Connection::_handle_response)
		{
			call<Rpc_sigh>(_response_io_handler);
		}

		enum class Execute_error { REQUEST_TOO_LARGE, RESPONSE_TOO_LARGE };

		/**
		 * Execute Forth code snippet at the system-control processor
		 *
		 * \param request_fn   functor that takes the designated buffer for
		 *                     the supplied code as 'char *, size_t' argument
		 *                     and returns the marshalled size of the code
		 *                     in bytes.
		 *
		 * \param response_fn  functor that takes the result of the SCP call
		 *                     as 'char const *', size_t' argument.
		 *
		 * \param error_fn     functor that is called with 'Execute_error'
		 *                     as argument whenever the SCP call failed.
		 */
		template <typename REQUEST_FN, typename RESPONSE_FN, typename ERROR_FN>
		void execute(REQUEST_FN  const &request_fn,
		             RESPONSE_FN const &response_fn,
		             ERROR_FN    const &error_fn)
		{
			/* marshal SCP program into shared buffer */
			size_t const len = request_fn(_ds.local_addr<char>(), MAX_REQUEST_LEN);

			/* schedule execution at the server */
			Request_result const request_result = call<Rpc_request>(len);
			if (request_result == Request_error::TOO_LARGE) {
				error_fn(Execute_error::REQUEST_TOO_LARGE);
				return;
			}

			Response_result response_result = Response_error::UNKNOWN;

			while (response_result == Response_error::UNKNOWN) {

				/* block for response signal */
				unsigned const orig_count = _response_count;
				while (orig_count == _response_count)
					_ep.wait_and_dispatch_one_io_signal();

				/* consume response */
				response_result = call<Rpc_response>();

				response_result.with_result(

					[&] (Response response) {
						response_fn(_ds.local_addr<char>(), response.bytes); },

					[&] (Response_error e) {
						switch (e) {
							case Response_error::UNKNOWN:
								warning("spurious SCP response signal");
								break;
							case Response_error::TOO_LARGE:
								warning("SCP response too large");
								break;
						}
					}
				);
			}
			if (response_result == Response_error::TOO_LARGE)
				error_fn(Execute_error::RESPONSE_TOO_LARGE);
		}
};

#endif /* _INCLUDE__SCP_SESSION__CONNECTION_H_ */
