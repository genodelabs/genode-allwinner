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
