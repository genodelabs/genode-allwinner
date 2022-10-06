/*
 * \brief  System-control processor (SCP) session interface
 * \author Norman Feske
 * \date   2022-05-11
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__SCP_SESSION__SCP_SESSION_H_
#define _INCLUDE__SCP_SESSION__SCP_SESSION_H_

#include <base/signal.h>
#include <session/session.h>
#include <dataspace/capability.h>

namespace Scp {

	using namespace Genode;

	enum class Execute_error { REQUEST_TOO_LARGE, RESPONSE_TOO_LARGE };

	class Session;
}


class Scp::Session : public Genode::Session
{
	public:

		/**
		 * \noapi
		 */
		static const char *service_name() { return "Scp"; }

		/*
		 * An SCP session consumes a dataspace capability for the server's
		 * session-object allocation, a session capability, and a dataspace
		 * capability for the shared request/response buffer.
		 */
		enum { CAP_QUOTA = 3 };

	protected:

		struct Request { };
		enum class Request_error { TOO_LARGE };
		using Request_result = Attempt<Request, Request_error>;

		struct Response { size_t bytes; };
		enum class Response_error { UNKNOWN, TOO_LARGE };
		using Response_result = Attempt<Response, Response_error>;

		static constexpr size_t MAX_REQUEST_LEN  = 1000;
		static constexpr size_t MAX_RESPONSE_LEN = 1000;

		virtual Request_result  _request(size_t) = 0;
		virtual Response_result _response() = 0;

		/*
		 * Utility used by 'Scp::Connection' and for issuing SCP calls locally
		 * by the SCP driver.
		 *
		 * The 'response_count' is expected to be incremented by the signal
		 * handler. It is used to detect the occurrence of a response.
		 */
		template <typename REQUEST_FN, typename RESPONSE_FN, typename ERROR_FN>
		void _execute(Entrypoint        &ep,
		              Byte_range_ptr     io_buffer_ptr,
		              unsigned    const &response_count,
		              REQUEST_FN  const &request_fn,
		              RESPONSE_FN const &response_fn,
		              ERROR_FN    const &error_fn)
		{
			/* marshal SCP program into shared buffer */
			size_t const len = request_fn(io_buffer_ptr.start, io_buffer_ptr.num_bytes);

			/* schedule execution at the server */
			Request_result const request_result = _request(len);
			if (request_result == Request_error::TOO_LARGE) {
				error_fn(Execute_error::REQUEST_TOO_LARGE);
				return;
			}

			Response_result response_result = Response_error::UNKNOWN;

			while (response_result == Response_error::UNKNOWN) {

				/* block for response signal */
				unsigned const orig_count = response_count;
				while (orig_count == response_count)
					ep.wait_and_dispatch_one_io_signal();

				/* consume response */
				response_result = _response();

				response_result.with_result(

					[&] (Response response) {
						response_fn(io_buffer_ptr.start, response.bytes); },

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

	public:

		/*********************
		 ** RPC declaration **
		 *********************/

		GENODE_RPC(Rpc_dataspace, Dataspace_capability, _dataspace);
		GENODE_RPC(Rpc_sigh,      void, _sigh, Signal_context_capability);
		GENODE_RPC(Rpc_request,   Request_result, _request, size_t);
		GENODE_RPC(Rpc_response,  Response_result, _response);

		GENODE_RPC_INTERFACE(Rpc_dataspace, Rpc_sigh, Rpc_request, Rpc_response);
};

#endif /* _INCLUDE__SCP_SESSION__SCP_SESSION_H_ */
