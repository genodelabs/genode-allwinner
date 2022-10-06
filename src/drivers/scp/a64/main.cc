/*
 * \brief  Allwinner A64 driver for interacting with the system-control processor
 * \author Norman Feske
 * \date   2022-05-06
 *
 * SCP is user 0, ARM is user 1
 *
 * Channel 0: ARM -> SCP
 * Channel 1: SCP -> ARM
 */

/* Genode includes */
#include <base/component.h>
#include <base/session_object.h>
#include <base/attached_ram_dataspace.h>
#include <base/heap.h>
#include <scp_session/scp_session.h>
#include <root/component.h>
#include <platform_session/device.h>
#include <platform_session/dma_buffer.h>

namespace Scp {

	using namespace Genode;

	struct Mbox_mmio;
	struct Session_component;
	struct Local_connection;
	struct Root;
	struct Main;

	using Sessions = Registry<Registered<Session_component> >;

	struct Seq_number { unsigned value; };

	struct Scheduler : Interface { virtual void schedule() = 0; };

	/**
	 * Buffer within SRAM-A2, shared between SCP and ARM
	 */
	struct Shared_buffer
	{
		uint32_t count;

		enum { CAPACITY = 1024u - sizeof(count) };
		char chars[CAPACITY];

	} __attribute__((packed));

	static_assert(sizeof(Shared_buffer) == 1024);
}


struct Scp::Mbox_mmio : Platform::Device::Mmio
{
	struct Ctrl0 : Register<0x0, 32>
	{
		enum { USER_SCP = 0, USER_ARM = 1 };
		struct Reception_mq0 : Bitfield<0,  1> { };
		struct Transmit_mq0  : Bitfield<4,  1> { };
		struct Reception_mq1 : Bitfield<8,  1> { };
		struct Transmit_mq1  : Bitfield<12, 1> { };
	};

	struct Irq_en : Register<0x60, 32>
	{
		struct Receive_mq1 : Bitfield<2,1> { };
	};

	struct Irq_status : Register<0x70, 32>
	{
		struct Receive_mq1 : Bitfield<2,1> { };
	};

	struct Queue_mq0 : Register<0x180, 32> { };
	struct Queue_mq1 : Register<0x184, 32> { };

	uint32_t status()         { return read<Irq_status>(); }
	bool     scp_has_value()  { return read<Irq_status::Receive_mq1>(); }
	uint32_t value_from_scp() { return read<Queue_mq1>(); }

	void value_to_scp(uint32_t value)
	{
		write<Queue_mq0>(value);
	}

	void clear_status()
	{
		write<Irq_status::Receive_mq1>(1);
	}

	Mbox_mmio(Platform::Device &device) : Platform::Device::Mmio(device)
	{
		/* configure channels 0 (ARM -> SCP) and 1 (SCP -> ARM) */
		write<Ctrl0::Reception_mq0>(Ctrl0::USER_SCP);
		write<Ctrl0::Transmit_mq0> (Ctrl0::USER_ARM);
		write<Ctrl0::Reception_mq1>(Ctrl0::USER_ARM);
		write<Ctrl0::Transmit_mq1> (Ctrl0::USER_SCP);

		/* enable interrupts for incoming messages */
		write<Irq_en::Receive_mq1>(1);

		clear_status();
	}
};


struct Scp::Session_component : Session_object<Scp::Session, Session_component>
{
	Env &_env;

	Scheduler &_scheduler;

	enum { DS_SIZE = 4096UL };

	Attached_ram_dataspace _ds { _env.ram(), _env.rm(), DS_SIZE };

	size_t _request_len = 0;

	struct Not_executed { };
	using Exec_id = Attempt<Seq_number, Not_executed>;

	/* make protected 'Scp::Session' types available to 'Main' */
	using Response_error  = Scp::Session::Response_error;
	using Response_result = Scp::Session::Response_result;
	using Response        = Scp::Session::Response;

	Response_result _response_result = Response_error::UNKNOWN;

	unsigned _response_count = 0;  /* solely used for 'local_execute' */

	Signal_context_capability _response_sigh { };

	Session_component(Env             &env,
	                  Resources const &resources,
	                  Label     const &label,
	                  Diag      const &diag,
	                  Scheduler       &scheduler)
	:
		Session_object(env.ep(), resources, label, diag),
		_env(env), _scheduler(scheduler)
	{
		_cap_quota_guard().withdraw(Cap_quota{1});
		_ram_quota_guard().withdraw(Ram_quota{DS_SIZE});
	}

	/*
	 * Identifier for the request while executed by the SCP. The number is
	 * allocated by the scheduler when picking up the request from the session.
	 * It is transmitted via the mbox channel 0 to the SCP and reflected by the
	 * SCP via mbox channel 1 upon completion. The response can thereby be
	 * associated to the corresponding session.
	 */
	Exec_id _exec_id = Not_executed { };

	template <typename FN>
	void with_pending_request(FN const &fn)
	{
		if (_request_len == 0)
			return;

		_exec_id = fn(_ds.local_addr<char>(), _request_len);

		/* don't try to handle the same request twice */
		_request_len = 0;
	}

	bool matches(Seq_number seq_number) const
	{
		return _exec_id.convert<bool>(
			[&] (Seq_number n) { return n.value == seq_number.value; },
			[&] (Not_executed) { return false; });
	}

	template <typename FN>
	void deliver_response(FN const &fn)
	{

		_response_result = fn(_ds.local_addr<char>(), MAX_RESPONSE_LEN);
		_response_count++;

		if (_response_sigh.valid())
			Signal_transmitter(_response_sigh).submit();

		_exec_id = Not_executed { };
	}

	/**
	 * Execute locally provided SCP program, used by 'Local_connection'
	 */
	template <typename REQUEST_FN, typename RESPONSE_FN, typename ERROR_FN>
	void local_execute(REQUEST_FN  const &request_fn,
	                   RESPONSE_FN const &response_fn,
	                   ERROR_FN    const &error_fn)
	{
		_execute(_env.ep(),
		         Byte_range_ptr(_ds.local_addr<char>(), MAX_REQUEST_LEN),
		         _response_count,
		         request_fn, response_fn, error_fn);
	}


	/*******************
	 ** RPC interface **
	 *******************/

	Dataspace_capability _dataspace() { return _ds.cap(); }

	void _sigh(Signal_context_capability sigh) { _response_sigh = sigh; }

	Request_result _request(size_t len) override
	{
		if (len > MAX_REQUEST_LEN)
			return Request_error::TOO_LARGE;

		_request_len     = len;
		_response_result = Response_error::UNKNOWN;

		_scheduler.schedule();

		return Request { };
	}

	Response_result _response() override { return _response_result; }
};


struct Scp::Local_connection : Noncopyable
{
	Session::Resources _resources { .ram_quota = { 20*1024 },
	                                .cap_quota = { Scp::Session::CAP_QUOTA } };

	Registered<Session_component> _session;

	Local_connection(Env &env, Sessions &sessions, Scheduler &scheduler)
	:
		_session(sessions, env, _resources, "local", Session::Diag { }, scheduler)
	{ }

	template <typename REQUEST_FN, typename RESPONSE_FN, typename ERROR_FN>
	void execute(REQUEST_FN  const &request_fn,
	             RESPONSE_FN const &response_fn,
	             ERROR_FN    const &error_fn)
	{
		_session.local_execute(request_fn, response_fn, error_fn);
	}
};


struct Scp::Root : Root_component<Session_component>
{
	Env       &_env;
	Sessions  &_sessions;
	Scheduler &_scheduler;

	Session_component *_create_session(const char *args) override
	{
		Session_component &session = *new (md_alloc())
			Registered<Session_component>(_sessions,
			                              _env,
			                              session_resources_from_args(args),
			                              label_from_args(args),
			                              session_diag_from_args(args),
			                              _scheduler);
		return &session;
	}

	void _destroy_session(Session_component *session) override
	{
		Genode::destroy(md_alloc(), session);
	}

	Root(Env &env, Allocator &md_alloc, Sessions &sessions, Scheduler &scheduler)
	:
		Root_component<Session_component>(env.ep(), md_alloc),
		_env(env), _sessions(sessions), _scheduler(scheduler)
	{ }
};


struct Scp::Main : private Scheduler
{
	Env &_env;

	Sliced_heap _sliced_heap { _env.ram(), _env.rm() };

	Sessions _sessions { };

	Root _root { _env, _sliced_heap, _sessions, *this };

	Platform::Connection   _platform  { _env };
	Platform::Device       _mbox      { _platform, "mbox"  };
	Mbox_mmio              _mmio      { _mbox };
	Platform::Device       _sram      { _platform, "sram a2" };
	Platform::Device::Mmio _sram_mmio { _sram };
	Platform::Device::Irq  _irq       { _mbox };

	Io_signal_handler<Main> _irq_handler { _env.ep(), *this, &Main::_handle_irq };

	Seq_number _last_submit   { };
	Seq_number _last_response { };

	enum class State { IDLE, BUSY };

	State _state { State::IDLE };

	template <typename FN>
	void _with_session(Seq_number seq_number, FN const &fn)
	{
		_sessions.for_each([&] (Session_component &session) {
			if (session.matches(seq_number))
				fn(session); });
	}

	void _handle_irq()
	{
		_mmio.clear_status();
		_irq.ack();

		if (_mmio.scp_has_value()) {
			_last_response.value = _mmio.value_from_scp();

			using Response_result = Session_component::Response_result;
			using Response_error  = Session_component::Response_error;
			using Response        = Session_component::Response;

			/* try to deliver response to client */
			_with_session(_last_response, [&] (Session_component &session) {
				session.deliver_response([&] (char *dst, size_t dst_size) {
					return _retrieve_from_scp(dst, dst_size).convert<Response_result>(
						[&] (Retrieval retrieval) {
							return Response { retrieval.len }; },
						[&] (Retrieve_error e) {
							switch (e) {
							case Retrieve_error::CAPACITY: break;
							}
							return Response_error::TOO_LARGE;
						}
					);
				});
			});
		}

		_state = State::IDLE;

		/* try to submit another pending request */
		schedule();
	}

	/* the SCP mbox input/output buffers are at the last 2 KiB of the SRAM A2 */
	addr_t const _sram_a2_end = (addr_t)(_sram_mmio.local_addr<char>() + 0x10000);

	Shared_buffer       &_mib = *(Shared_buffer *)(_sram_a2_end - 0x400);
	Shared_buffer const &_mob = *(Shared_buffer *)(_sram_a2_end - 0x800);

	/**
	 * Invert the lowest two bits of a given byte index
	 *
	 * The SCP uses big endian whereas the ARM uses little endian.
	 * To ease the access of the SCP to 32-bit hardware registers, the
	 * SCP bus logic swizzles the lowest two address bits. This mechanism,
	 * however, interferes with byte-wise bus accesses. Hence, when copying
	 * byte buffers between the SCP and ARM, we have to swizzle the content
	 * such that the SCP's swizzling logic results in the original content.
	 */
	static unsigned _swizzled_index(unsigned i) { return i + 3 - 2*(i & 3); }

	enum class Submit_error { BUSY, CAPACITY };
	using Submit_result = Attempt<Seq_number, Submit_error>;

	Submit_result _submit_to_scp(char const *str, size_t len)
	{
		if (_state != State::IDLE)
			return Submit_error::BUSY;

		if (len > Shared_buffer::CAPACITY)
			return Submit_error::CAPACITY;

		for (unsigned i = 0; i < len; i++)
			_mib.chars[_swizzled_index(i)] = str[i];

		_mib.count = len & 0x3ffu;

		_last_submit.value++;
		_mmio.value_to_scp(_last_submit.value);
		_state = State::BUSY;

		return _last_submit;
	}

	struct Retrieval { size_t len; };
	enum class Retrieve_error { CAPACITY };
	using Retrieve_result = Attempt<Retrieval, Retrieve_error>;

	Retrieve_result _retrieve_from_scp(char *dst, size_t dst_size)
	{
		size_t const len = _mob.count;
		bool   const mob_saturated = (len >= 1000u);

		/* consider fully saturated mob as overflown */
		if (mob_saturated || len > dst_size)
			return Retrieve_error::CAPACITY;

		for (unsigned i = 0; i < len; i++)
			dst[i] = _mob.chars[_swizzled_index(i)];

		return Retrieval { len };
	}

	/**
	 * Scheduler interface
	 *
	 * Try to submit a pending request to SCP
	 */
	void schedule() override
	{
		_sessions.for_each([&] (Session_component &session) {

			if (_state != State::IDLE)
				return;

			session.with_pending_request([&] (char const *request, size_t len) {

				using Exec_id = Session_component::Exec_id;

				return _submit_to_scp(request, len).convert<Exec_id>(
					[&] (Seq_number id) { return id; },
					[&] (Submit_error e) {
						switch (e) {
						case Submit_error::BUSY:
							error("attempted to submit request to busy SCP");
							break;
						case Submit_error::CAPACITY:
							error("SCP request exceeds maximum capacity");
							break;
						}
						return Session_component::Not_executed { };
					}
				);
			});
		});
	}

	Main(Env &env) : _env(env)
	{
		_irq.sigh(_irq_handler);
		_env.parent().announce(_env.ep().manage(_root));
	}
};


void Component::construct(Genode::Env &env)
{
	static Scp::Main main(env);
}
