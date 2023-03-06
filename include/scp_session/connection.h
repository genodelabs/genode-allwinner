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
	private:

		Entrypoint &_ep;

		Attached_dataspace _ds;

		Io_signal_handler<Connection> _response_io_handler;

		unsigned _response_count = 0;

		void _handle_response() { _response_count++; }

		Request_result _request(size_t len) override
		{
			return call<Rpc_request>(len);
		}

		Response_result _response() override
		{
			return call<Rpc_response>();
		}

	public:

		Connection(Env &env, Label const &label = Label())
		:
			Genode::Connection<Scp::Session>(env, label,
			                                 Ram_quota { 20*1024 }, Args()),
			Rpc_client<Session>(cap()),
			_ep(env.ep()),
			_ds(env.rm(), call<Rpc_dataspace>()),
			_response_io_handler(_ep, *this, &Connection::_handle_response)
		{
			call<Rpc_sigh>(_response_io_handler);
		}

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
			_execute(_ep,
			         Byte_range_ptr(_ds.local_addr<char>(), MAX_REQUEST_LEN),
			         _response_count,
			         request_fn, response_fn, error_fn);
		}
};

#endif /* _INCLUDE__SCP_SESSION__CONNECTION_H_ */
