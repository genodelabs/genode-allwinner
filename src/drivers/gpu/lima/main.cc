/*
 * \brief  Lima GPU driver Linux port
 * \author Josef Soentgen
 * \date   2021-10-28
 */

/*
 * Copyright (C) 2021-2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

/* Genode includes */
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/env.h>
#include <base/heap.h>
#include <base/id_space.h>
#include <base/session_object.h>
#include <base/signal.h>
#include <base/sleep.h>
#include <gpu/info_lima.h>
#include <gpu_session/gpu_session.h>
#include <root/component.h>
#include <session/session.h>

/* emulation includes */
#include <lx_emul/init.h>
#include <lx_emul/task.h>
#include <lx_kit/env.h>
#include <lx_kit/init.h>


/* local includes */
#include "lx_drm.h"

extern Genode::Dataspace_capability genode_lookup_cap(void *, unsigned long long, unsigned long);

namespace Gpu {

	using namespace Genode;

	struct Session_component;
	using Session_space = Genode::Id_space<Session_component>;
	struct Root;

	/*
	 * Allow multiple clients are allowed the driver must currently only used by one
	 * client at a time. The client, however, may open multiple connections as those
	 * implicitly provide a new context and sync objects.
	 */
	using Root_component = Genode::Root_component<Session_component, Genode::Multiple_clients>;

	struct Operation;
	struct Request;
	struct Local_request;

	struct Buffer_space;
	struct Worker_args;

	struct Ctx_id;
	struct Syncobj_id;

	struct Buffer;
	using Buffer_id = Vram_id;

} /* namespace Gpu */

struct Gpu::Vram { };


struct Gpu::Ctx_id
{
	uint32_t value;
};


struct Gpu::Syncobj_id
{
	uint32_t value;
};


struct Gpu::Operation
{
	enum class Type {
		INVALID    = 0,
		ALLOC      = 1,
		FREE       = 2,
		EXEC       = 5,
		WAIT       = 6,
		CTX_CREATE = 7,
		CTX_FREE   = 8,
		SYNCOBJ_CREATE  = 9,
		SYNCOBJ_DESTROY = 10,
		SYNCOBJ_WAIT    = 11,
	};

	Type type;

	uint32_t           size;
	uint32_t           va;
	Buffer_id          id;
	Sequence_number    seqno;

	Ctx_id ctx_id;
	uint32_t op;

	enum { MAX_PIPE = 2 };
	Syncobj_id syncobj_id[MAX_PIPE];

	bool valid() const
	{
		return type != Type::INVALID;
	}

	static char const *type_name(Type type)
	{
		switch (type) {
		case Type::INVALID:    return "INVALID";
		case Type::ALLOC:      return "ALLOC";
		case Type::FREE:       return "FREE";
		case Type::EXEC:       return "EXEC";
		case Type::WAIT:       return "WAIT";
		case Type::CTX_CREATE: return "CTX_CREATE";
		case Type::CTX_FREE:   return "CTX_FREE";
		case Type::SYNCOBJ_CREATE:  return "SYNCOBJ_CREATE";
		case Type::SYNCOBJ_DESTROY: return "SYNCOBJ_DESTROY";
		case Type::SYNCOBJ_WAIT:    return "SYNCOBJ_WAIT";
		}
		return "INVALID";
	}

	void print(Genode::Output &out) const
	{
		Genode::print(out, type_name(type));
	}
};


struct Gpu::Request
{
	struct Tag { unsigned long value; };

	Operation operation;

	bool success;

	Tag tag;

	bool valid() const
	{
		return operation.valid();
	}

	void print(Genode::Output &out) const
	{
		Genode::print(out, "tag=",       tag.value, " "
		                   "success=",   success,   " "
		                   "operation=", operation);
	}

	static Gpu::Request create(Operation::Type type)
	{
		static unsigned long tag_counter = 0;

		return Gpu::Request {
			.operation = Operation {
				.type = type,
				.size = 0,
				.va = 0,
				.id = Buffer_id { .value = 0 },
				.seqno = Sequence_number { .value = 0 },
				.ctx_id = Ctx_id { .value = 0 },
				.op = 0,
				.syncobj_id = {
					Syncobj_id { .value = 0 },
					Syncobj_id { .value = 0 },
				},
			},
			.success = false,
			.tag = Tag { ++tag_counter }
		};
	}

	bool matches(Gpu::Request const &r) const
	{
		return tag.value == r.tag.value;
	}
};


struct Gpu::Local_request
{
	enum class Type { INVALID = 0, OPEN, CLOSE };
	Type type;
	bool success;
};


struct Gpu::Buffer : Vram
{
	Vram_id_space::Element const _elem;

	uint32_t             const handle;
	Dataspace_capability const cap;
	Attached_dataspace         attached_ds;

	uint32_t const va;

	Buffer(Gpu::Vram_id_space  &space,
	       Gpu::Buffer_id       id,
	       uint32_t             handle,
	       uint32_t             va,
	       Dataspace_capability cap,
	       Region_map &rm)
	:
		_elem       { *this, space, id },
		handle      { handle },
		cap         { cap },
		attached_ds { rm, cap },
		va          { va }
	{ }

};


extern "C" void lx_emul_mem_cache_clean_invalidate(const void * addr,
                                                   unsigned long size);

struct Gpu::Buffer_space : Vram_id_space
{
	Allocator &_alloc;

	Buffer_space(Allocator &alloc) : _alloc { alloc } { }

	~Buffer_space() { }

	void *local_addr(Gpu::Buffer_id id)
	{
		void *local_addr = nullptr;
		apply<Buffer>(id, [&] (Buffer &b) {
			local_addr = b.attached_ds.local_addr<void>();
		});

		return local_addr;
	}

	struct Lx_handle
	{
		uint32_t value;
		bool     _valid;

		bool valid() const { return _valid; }
	};

	Lx_handle lookup_and_flush(Gpu::Buffer_id id, bool flush)
	{
		Lx_handle result { 0, false };

		apply<Buffer>(id, [&] (Buffer &b) {

			if (flush)
				lx_emul_mem_cache_clean_invalidate(b.attached_ds.local_addr<void>(),
				                                   b.attached_ds.size());

			result = { b.handle, true };
		});

		return result;
	}

	void insert(Gpu::Buffer_id id, uint32_t handle, uint32_t va,
	            Dataspace_capability cap, Region_map &rm)
	{
		// XXX assert id is not assosicated with other handle and
		//     handle is not already present in registry
		new (&_alloc) Buffer(*this, id, handle, va, cap, rm);
	}

	void remove(Gpu::Buffer_id id)
	{
		bool removed = false;
		apply<Buffer>(id, [&] (Buffer &b) {
			destroy(_alloc, &b);
			removed = true;
		});

		if (!removed)
			Genode::warning("could not remove buffer with id: ", id.value,
			                " - not present in registry");
	}

	Dataspace_capability lookup_buffer(Gpu::Buffer_id id)
	{
		Dataspace_capability cap { };
		try {
			apply<Buffer>(id, [&] (Buffer const &b) {
				cap = b.cap;
			});
		} catch (Vram_id_space::Unknown_id) { }
		return cap;
	}

	template <typename FN>
	void with_va(Gpu::Buffer_id id, FN const &fn)
	{
		apply<Buffer>(id, [&] (Buffer const &b) {
			fn(b.va);
		});
	}

	template <typename FN>
	void with_handle(Gpu::Buffer_id id, FN const &fn)
	{
		apply<Buffer>(id, [&] (Buffer const &b) {
			fn(b.handle);
		});
	}

	bool managed(Gpu::Buffer_id id)
	{
		bool result = false;
		apply<Buffer>(id, [&] (Buffer const &) {
			result = true;
		});
		return result;
	}
};


struct Syncobj_notifier : Genode::Interface
{
	virtual void notify() = 0;
};


struct Gpu::Worker_args
{
	Syncobj_notifier *syncobj_notifier { nullptr };

	void signal_syncobj_wait(void)
	{
		if (syncobj_notifier)
			syncobj_notifier->notify();
	}

	Region_map *rm { nullptr };

	Gpu::Request *pending_request   { nullptr };
	Gpu::Request *completed_request { nullptr };

	Gpu::Local_request *local_request { nullptr };

	void *drm { nullptr };

	Gpu::Info_lima *info { nullptr };

	Buffer_space *buffers { nullptr };

	void *gem_submit { nullptr };

	bool valid() const
	{
		return buffers != nullptr && info != nullptr;
	}

	template <typename FN> void for_each_pending_request(FN const &fn)
	{
		if (!pending_request || !pending_request->valid()) {
			return;
		}

		/*
 		 * Reset first to prevent _schedule_request from picking up
		 * unfinished requests, e.g. SYNCOBJ_WAIT.
		 */
		*completed_request = Gpu::Request();

		*completed_request = fn(*pending_request);
		*pending_request   = Gpu::Request();
	}
};


static int _populate_info(void *drm, Gpu::Info_lima &info)
{
	for (Gpu::Info_lima::Param &p : info.param)
		p = 0;

	Genode::uint8_t params[Gpu::Info_lima::MAX_LIMA_PARAMS] = {
		0x00, /* DRM_LIMA_PARAM_GPU_ID */
		0x01, /* DRM_LIMA_PARAM_NUM_PP */
		0x02, /* DRM_LIMA_PARAM_GP_VERSION */
		0x03, /* DRM_LIMA_PARAM_PP_VERSION */
	};

	for (int p = 0; p < Gpu::Info_lima::MAX_LIMA_PARAMS; p++) {

		Genode::uint64_t value;
		int const err = lx_drm_ioctl_lima_gem_param(drm, params[p], &value);
		if (err)
			return -1;

		info.param[p] = value;
	}
	return 0;
}


static Gpu::Worker_args    _worker_args;
extern struct task_struct *_lx_user_task;

extern "C" void *lx_user_task_args;


extern "C" int run_lx_user_task(void *p)
{
	Gpu::Worker_args &args = *static_cast<Gpu::Worker_args*>(p);

	using namespace Genode;
	using OP = Gpu::Operation::Type;

	int count_drm_session = 0;

	while (true) {

		/* wait until we have a valid session */
		if (!args.valid()) {
			lx_emul_task_schedule(true);
			continue;
		}

		Gpu::Buffer_space &buffers = *args.buffers;
		Region_map        &rm      = *args.rm;

		/* handle local requests first */
		if (args.local_request) {
			args.local_request->success = false;
			switch (args.local_request->type) {
			case Gpu::Local_request::Type::OPEN:
				if (!args.drm) {
					args.drm = lx_drm_open();
					if (!args.drm)
						break;
				}

				if (args.drm) {
					++count_drm_session;
					_populate_info(args.drm, *args.info);

					args.local_request->success = true;
				}
				break;
			case Gpu::Local_request::Type::CLOSE:
				--count_drm_session;
				if (count_drm_session <= 0) {
					lx_drm_close(args.drm);
					args.drm = nullptr;
				}
				args.local_request->success = true;
				break;
			case Gpu::Local_request::Type::INVALID:
				break;
			}
		}

		bool notify_client = false;

		auto dispatch_pending = [&] (Gpu::Request r) {

			/* clear request result */
			r.success = false;

			switch (r.operation.type) {
			case OP::CTX_CREATE:
			{
				unsigned int id;
				int err = lx_drm_ioctl_lima_ctx_create(args.drm, &id);
				if (err) {
					error("lx_drm_ioctl_lima_ctx_create failed: ", err);
					break;
				}

				r.operation.ctx_id = Gpu::Ctx_id { .value = id };

				r.success = true;
				break;
			}
			case OP::CTX_FREE:
			{
				unsigned int const id = r.operation.ctx_id.value;
				int err =
					lx_drm_ioctl_lima_ctx_free(args.drm, id);
				if (err) {
					error("lx_drm_ioctl_lima_ctx_free failed: ", err);
					break;
				}

				r.success = true;
				break;
			}
			case OP::SYNCOBJ_CREATE:
			{
				unsigned int handle;
				int err =
					lx_drm_ioctl_syncobj_create(args.drm, &handle);
				if (err) {
					error("lx_drm_ioctl_syncobj_create failed: ", err);
					break;
				}

				r.operation.syncobj_id[0] =
					Gpu::Syncobj_id { .value = handle };

				r.success = true;
				break;
			}
			case OP::SYNCOBJ_DESTROY:
			{
				unsigned int const handle = r.operation.syncobj_id[0].value;
				int err =
					lx_drm_ioctl_syncobj_destroy(args.drm, handle);
				if (err) {
					error("lx_drm_ioctl_syncobj_destroy failed: ", err);
					break;
				}

				r.success = true;
				break;
			}
			case OP::SYNCOBJ_WAIT:
			{
				unsigned int  const handle = r.operation.syncobj_id[0].value;

				/* results < 0 denotes errors, == 0 success and > 0 timeouts */
				int err =
					lx_drm_ioctl_syncobj_wait(args.drm, handle);

				notify_client = true;

				if (err < 0) {
					error("lx_drm_ioctl_syncobj_wait ", handle, " failed: ", err);
					break;
				}

				if (err > 0)
					break;

				r.success = true;
				break;
			}
			case OP::ALLOC:
			{
				uint32_t const size = r.operation.size;
				uint32_t va = r.operation.va;
				uint32_t handle;

				int err =
					lx_drm_ioctl_lima_gem_create(args.drm, va, size, &handle);
				if (err) {
					error("lx_drm_ioctl_lima_gem_create failed: ", err);
					break;
				}

				unsigned int       va_allocated;
				unsigned long long offset;
				err = lx_drm_ioctl_lima_gem_info(args.drm, handle, &va_allocated, &offset);
				if (err) {
					error("lx_drm_ioctl_lima_gem_info failed: ", err);
					lx_drm_ioctl_gem_close(args.drm, handle);
					break;
				}

				if (va != va_allocated) {
					error("va wrong ", Hex(va), " != ", Hex(va_allocated));
					lx_drm_ioctl_gem_close(args.drm, handle);
					break;
				}

				Dataspace_capability cap =
					genode_lookup_cap(args.drm, offset, size);
				buffers.insert(r.operation.id, handle, va, cap, rm);

				r.success = true;
				break;
			}
			case OP::FREE:
			{
				bool found = false;
				buffers.with_handle(r.operation.id, [&] (uint32_t const handle) {
					(void)lx_drm_ioctl_gem_close(args.drm, handle);
					found = true;
				});

				if (found) {
					buffers.remove(r.operation.id);
					r.success = true;
				}
				break;
			}
			case OP::EXEC:
			{
				void *gem_submit = buffers.local_addr(r.operation.id);
				if (!gem_submit)
					break;

				int err = 0;
				unsigned nr_bos = lx_drm_gem_submit_bo_count(gem_submit);
				for (unsigned i = 0; i < nr_bos; i++) {
					unsigned *bo_handle = lx_drm_gem_submit_bo_handle(gem_submit, i);
					bool const bo_read  = lx_drm_gem_submit_bo_read(gem_submit, i);
					if (!bo_handle) {
						error("lx_drm_gem_submit_bo_handle: index: ", i,
						       " invalid bo handle");
						err = -1;
						break;
					}
					using LX = Gpu::Buffer_space::Lx_handle;
					Gpu::Buffer_id id { .value = *bo_handle };
					/* flush only when read by the GPU */
					LX handle = buffers.lookup_and_flush(id, bo_read);
					if (!handle.valid()) {
						error("could not look up handle for id: ", *bo_handle);
						err = -1;
						break;
					}
					/* replace client-local buffer id with kernel-local handle */
					*bo_handle = handle.value;
				}

				/* replace context id */
				lx_drm_gem_submit_ctx_id(gem_submit, r.operation.ctx_id.value);

				/* replace out_sync id */
				unsigned int const pipe = lx_drm_gem_submit_pipe(gem_submit);
				if (pipe >= Gpu::Operation::MAX_PIPE)
					break;
				lx_drm_gem_submit_set_out_sync(gem_submit, r.operation.syncobj_id[pipe].value);

				if (!err)
					err = lx_drm_ioctl_lima_gem_submit(args.drm,
					                                   (unsigned long)gem_submit);
				if (err) {
					error("lx_drm_ioctl_lima_gem_submit: ", err);
					break;
				}

				r.operation.seqno.value =
					lx_drm_gem_submit_out_sync(gem_submit);
				r.success = true;

				break;
			}
			case OP::WAIT:
			{
				buffers.with_handle(r.operation.id, [&] (uint32_t const handle) {

					int const err = lx_drm_ioctl_lima_gem_wait(args.drm, handle,
					                                           r.operation.op);
					notify_client = true;
					if (err)
						return;

					r.success = true;
				});
				break;
			}
			default:
			break;
			}

			/* return completed request */
			return r;
		};

		args.for_each_pending_request(dispatch_pending);

		if (notify_client)
			args.signal_syncobj_wait();

		lx_emul_task_schedule(true);
	}
}


static void request_already_pending(Gpu::Request const&, Gpu::Request const&) __attribute__((noreturn));
static void request_already_pending(Gpu::Request const &p,
                                    Gpu::Request const &r)
{
	Genode::error("there is already a request pending: ", p,
	              ", cannot schedule request: ", r);
	Genode::sleep_forever();
}


struct Gpu::Session_component : public Genode::Session_object<Gpu::Session>
{
	private:

		Session_component(Session_component const&) = delete;
		Session_component& operator=(Session_component const&) = delete;

		Genode::Env        &_env;
		Genode::Entrypoint &_ep;

		Genode::Id_space<Session_component>::Element const _elem;

		Genode::Attached_ram_dataspace _info_dataspace {
			_env.ram(), _env.rm(), 4096 };
		Genode::Signal_context_capability _completion_sigh { };

		char const *_name;

		Gpu::Request _pending_request   { };
		Gpu::Request _completed_request { };

		Gpu::Worker_args &_lx_task_args;
		task_struct      *_lx_task;
		Gpu::Buffer_space &_buffers { *_lx_task_args.buffers };

		bool _managed_id(Gpu::Request const &request)
		{
			using OP = Gpu::Operation::Type;

			switch (request.operation.type) {
			case OP::FREE:  [[fallthrough]];
			case OP::EXEC:
				return _buffers.managed(request.operation.id);
			default:
				break;
			}

			return true;
		}

		enum class Blocking_request : uint8_t { NO, YES };

		template <typename SUCC_FN, typename FAIL_FN>
		void _schedule_request(Gpu::Request const &request,
		                       SUCC_FN const &succ_fn,
		                       FAIL_FN const &fail_fn,
		                       Blocking_request block = Blocking_request::YES)
		{
			if (_pending_request.valid())
				request_already_pending(_pending_request, request);

			/*
			 * Requests referencing not managed handles will be
			 * treated as scheduled but failed.
			 */
			if (!_managed_id(request)) {
				fail_fn();
				return;
			}

			_pending_request = request;
			_lx_task_args.pending_request   = &_pending_request;
			_lx_task_args.completed_request = &_completed_request;

			lx_emul_task_unblock(_lx_task);
			Lx_kit::env().scheduler.schedule();

			if (block == Blocking_request::YES)
				for (;;) {
					if (_completed_request.matches(request)
					 && _completed_request.valid())
						break;

					_ep.wait_and_dispatch_one_io_signal();
				}

			if (_completed_request.success)
				succ_fn(_completed_request);
			else
				fail_fn();
		}

		bool _local_request(Gpu::Local_request::Type type)
		{
			Gpu::Local_request local_request {
				.type = type,
				.success = false,
			};
			_lx_task_args.local_request = &local_request;

			// XXX must not return prematurely
			lx_emul_task_unblock(_lx_task);
			Lx_kit::env().scheduler.schedule();

			bool const success = _lx_task_args.local_request->success;
			_lx_task_args.local_request = nullptr;

			return success;
		}

		Gpu::Ctx_id _ctx_id { 0 };

		Gpu::Ctx_id _create_ctx()
		{
			Gpu::Ctx_id ctx_id { 0 };

			Gpu::Request r = Gpu::Request::create(Gpu::Operation::Type::CTX_CREATE);

			auto success = [&] (Gpu::Request const &request) {
				ctx_id = request.operation.ctx_id;
			};
			auto fail = [&] () { };
			_schedule_request(r, success, fail);

			return ctx_id;
		}

		void _free_ctx(Gpu::Ctx_id ctx_id)
		{
			Gpu::Request r = Gpu::Request::create(Gpu::Operation::Type::CTX_FREE);
			r.operation.ctx_id = ctx_id;

			auto success = [&] (Gpu::Request const &) { };
			auto fail = [&] () { warning("could not free ctx_id: ", ctx_id.value); };
			_schedule_request(r, success, fail);
		}

		Gpu::Syncobj_id _sync_id[Gpu::Operation::MAX_PIPE] { { 0 }, { 0 } };

		Gpu::Syncobj_id _create_syncobj()
		{
			Gpu::Syncobj_id syncobj_id { 0 };

			Gpu::Request r = Gpu::Request::create(Gpu::Operation::Type::SYNCOBJ_CREATE);

			auto success = [&] (Gpu::Request const &request) {
				syncobj_id = request.operation.syncobj_id[0];
			};
			auto fail = [&] () { };
			_schedule_request(r, success, fail);

			return syncobj_id;
		}

		void _destroy_syncobj(Gpu::Syncobj_id syncobj_id)
		{
			Gpu::Request r = Gpu::Request::create(Gpu::Operation::Type::SYNCOBJ_DESTROY);
			r.operation.syncobj_id[0] = syncobj_id;

			auto success = [&] (Gpu::Request const &) { };
			auto fail = [&] () { warning("could not destroy syncobj: ", syncobj_id.value); };
			_schedule_request(r, success, fail);
		}

		/*
		 * Waiting for an sync object and waiting for access
		 * to a given buffer-object is done by blocking the
		 * calling linux task.
		 */

		bool _pending_syncobj_wait { false };
		bool _pending_wait         { false };


	public:

		struct Could_not_open_drm : Genode::Exception { };

		/**
		 * Constructor
		 */
		Session_component(Genode::Env        &env,
		                  Genode::Entrypoint &ep,
		                  Resources    const &resources,
		                  Label        const &label,
		                  Diag                diag,
		                  char         const *name,
		                  Genode::Id_space<Session_component> &space)
		:
			Session_object { ep, resources, label, diag },
			_env         { env },
			_ep          { ep },
			_elem        { *this, space },
			_name        { name },
			_lx_task_args { _worker_args },
			_lx_task     { _lx_user_task }
		{
			if (!_local_request(Gpu::Local_request::Type::OPEN)) {
				Genode::warning("could not open DRM session");
				throw Could_not_open_drm();
			}

			_ctx_id = _create_ctx();

			_sync_id[0] = _create_syncobj();
			_sync_id[1] = _create_syncobj();

			void *info = _info_dataspace.local_addr<void>();
			Genode::memcpy(info, _lx_task_args.info, sizeof (Gpu::Info_lima));
		}

		virtual ~Session_component()
		{
			if (_lx_task_args.pending_request->valid()) {
				Genode::warning("destructor override currently pending request");
			}

			for (unsigned i = 0; i < Gpu::Operation::MAX_PIPE; i++) {
				_destroy_syncobj(_sync_id[i]);
			}

			_free_ctx(_ctx_id);

			if (!_local_request(Gpu::Local_request::Type::CLOSE))
				Genode::warning("could not close DRM session - leaking objects");
		}

		char const *name() { return _name; }

		void submit_completion_signal()
		{
			if (_completion_sigh.valid())
				Genode::Signal_transmitter(_completion_sigh).submit();
		}

		bool operation_pending() const
		{
			/* only notify sessions with pending operations */
			return _pending_syncobj_wait || _pending_wait;
		}

		/***************************
		 ** Gpu session interface **
		 ***************************/

		Genode::Dataspace_capability info_dataspace() const override
		{
			return _info_dataspace.cap();
		}

		Gpu::Sequence_number execute(Gpu::Vram_id id,
		                                 Genode::off_t) override
		{
			Gpu::Request r = Gpu::Request::create(Gpu::Operation::Type::EXEC);
			r.operation.id = id;
			r.operation.ctx_id = _ctx_id;
			r.operation.syncobj_id[0] = _sync_id[0];
			r.operation.syncobj_id[1] = _sync_id[1];

			Gpu::Sequence_number seqno { .value = 0 };

			auto success = [&] (Gpu::Request const &request) {
				seqno = request.operation.seqno;
			};
			auto fail = [&] () {
				throw Invalid_state();
			};
			_schedule_request(r, success, fail);

			return seqno;
		}

		bool complete(Gpu::Sequence_number seqno) override
		{
			bool valid = false;
			for (auto v : _sync_id) {
				if (seqno.value == v.value) {
					valid = true;
					break;
				}
			}
			if (!valid)
				return false;

			if (_pending_syncobj_wait) {
				Gpu::Request *pr = _lx_task_args.pending_request;
				if (pr && pr->operation.type == Gpu::Operation::Type::SYNCOBJ_WAIT) {
					return false;
				}

				Gpu::Request *cr = _lx_task_args.completed_request;
				if (cr && cr->operation.type == Gpu::Operation::Type::SYNCOBJ_WAIT) {
					_pending_syncobj_wait = false;
					return cr->success;
				}
			}

			Gpu::Request r = Gpu::Request::create(Gpu::Operation::Type::SYNCOBJ_WAIT);
			r.operation.syncobj_id[0] =
				Gpu::Syncobj_id { .value = (uint32_t)seqno.value };

			bool completed = false;
			_pending_syncobj_wait = false;

			auto success = [&] (Gpu::Request const &request) {
				completed = request.success;
			};
			auto fail = [&] () {
				_pending_syncobj_wait = true;
			};
			_schedule_request(r, success, fail, Blocking_request::NO);

			return completed;
		}

		void completion_sigh(Genode::Signal_context_capability sigh) override
		{
			_completion_sigh = sigh;
		}

		Genode::Dataspace_capability alloc_vram(Gpu::Vram_id,
		                                        Genode::size_t) override
		{
			Genode::warning(__func__, ": not implemented");
			return Dataspace_capability();
		}

		void free_vram(Gpu::Vram_id) override
		{
			Genode::warning(__func__, ": not implemented");
		}

		Genode::Dataspace_capability map_cpu(Gpu::Vram_id id,
		                                        Gpu::Mapping_attributes) override
		{
			return _buffers.lookup_buffer(id);
		}

		void unmap_cpu(Vram_id) override
		{
			Genode::warning(__func__, ": not implemented");
		}

		bool map_gpu(Vram_id id, Genode::size_t size, Genode::off_t,
		             Virtual_address va) override
		{
			Genode::Dataspace_capability cap { };

			cap = _buffers.lookup_buffer(id);
			if (cap.valid()) {
				error("Duplicate 'map_gpu' called for ", id.value);
				return false;
			}

			if (size > ~0U) {
				error("Allocation of buffers > 4G not supported!");
				return false;
			}

			Gpu::Request r = Gpu::Request::create(Gpu::Operation::Type::ALLOC);
			r.operation.id   = id;
			r.operation.va   = (uint32_t) va.va;
			r.operation.size = (uint32_t) size;

			bool ret = false;
			auto success = [&] (Gpu::Request const &) {
				ret = true;
			};
			auto fail = [&] () { };
			_schedule_request(r, success, fail);

			return ret;
		}

		void unmap_gpu(Vram_id id, Genode::off_t, Virtual_address) override
		{
			Gpu::Request r = Gpu::Request::create(Gpu::Operation::Type::FREE);
			r.operation.id = id;

			auto success = [&] (Gpu::Request const &) { };
			auto fail = [&] () { };
			_schedule_request(r, success, fail);
		}

		bool set_tiling_gpu(Gpu::Buffer_id id, Genode::off_t,  unsigned mode) override
		{
			if (_pending_wait) {
				Gpu::Request *pr = _lx_task_args.pending_request;
				if (pr && pr->operation.type == Gpu::Operation::Type::WAIT) {
					return false;
				}

				Gpu::Request *cr = _lx_task_args.completed_request;
				if (cr && cr->operation.type == Gpu::Operation::Type::WAIT) {
					_pending_wait = false;
					return cr->success;
				}
			}

			Gpu::Request r = Gpu::Request::create(Gpu::Operation::Type::WAIT);
			r.operation.id = id;
			r.operation.op = mode;

			bool completed = false;
			_pending_wait = false;

			auto success = [&] (Gpu::Request const &request) {
				completed = request.success;
			};
			auto fail = [&] () {
				_pending_wait = true;
			};
			_schedule_request(r, success, fail, Blocking_request::NO);

			return completed;
		}

		Gpu::Vram_capability export_vram(Gpu::Vram_id) override
		{
			Genode::warning(__func__, ": not implemented");
			return Gpu::Vram_capability();
		}

		void import_vram(Gpu::Vram_capability, Gpu::Vram_id) override
		{
			Genode::warning(__func__, ": not implemented");
		}
};


struct Gpu::Root : Gpu::Root_component
{
	private:

		/*
		 * Noncopyable
		 */
		Root(Root const &) = delete;
		Root &operator = (Root const &) = delete;

		Genode::Env       &_env;
		Genode::Allocator &_alloc;

		uint32_t           _session_id;

		Gpu::Session_space _session_space { };

	protected:

		Session_component *_create_session(char const *args) override
		{
			char *name = (char*)_alloc.alloc(64);
			Genode::String<64> tmp("gpu_worker-", ++_session_id);
			Genode::memcpy(name, tmp.string(), tmp.length());

			Session::Label const label  { session_label_from_args(args) };

			Session_component *sc =
				new (_alloc) Session_component(_env, _env.ep(),
			                                   session_resources_from_args(args),
			                                   label,
			                                   session_diag_from_args(args),
			                                   name,
			                                   _session_space);
			return sc;
		}

		void _upgrade_session(Session_component *sc, char const *args) override
		{
			sc->upgrade(ram_quota_from_args(args));
			sc->upgrade(cap_quota_from_args(args));
		}

		void _destroy_session(Session_component *sc) override
		{
			char const *name = sc->name();

			Genode::destroy(_alloc, const_cast<char*>(name));
			Genode::destroy(md_alloc(), sc);
		}

	public:

		Root(Genode::Env &env, Genode::Allocator &alloc)
		:
			Root_component { env.ep(), alloc },
			_env           { env },
			_alloc         { alloc },
			_session_id    { 0 }
		{ }

		void completion_signal()
		{
			_session_space.for_each<Session_component>(
				[&] (Session_component &sc) {
					if (sc.operation_pending())
						sc.submit_completion_signal();
				});
		}
};


static Genode::Constructible<Gpu::Root> _gpu_root { };


void lx_emul_announce_gpu_session(void)
{
	if (!_gpu_root.constructed()) {
		_gpu_root.construct(Lx_kit::env().env, Lx_kit::env().heap);

		Genode::Entrypoint &ep = Lx_kit::env().env.ep();
		Lx_kit::env().env.parent().announce(ep.manage(*_gpu_root));
	}
}


void lx_emul_submit_completion_signal(void)
{
	if (!_gpu_root.constructed())
		return;

	_gpu_root->completion_signal();
}


namespace Driver {

	using namespace Genode;

	struct Main;
} /* namespace Driver */


struct Driver::Main : private Entrypoint::Io_progress_handler
{
	Env                    &_env;
	Attached_rom_dataspace  _config_rom { _env, "config" };

	using Dtb_name = Genode::String<64>;
	Dtb_name              _dtb_name {
		_config_rom.xml().attribute_value("dtb", Dtb_name("dtb")) };
	Attached_rom_dataspace _dtb_rom { _env, _dtb_name.string() };

	Genode::Sliced_heap  _alloc          { _env.ram(), _env.rm() };
	Genode::Region_map  &_worker_rm      { _env.rm() };
	Gpu::Buffer_space    _worker_buffers { _alloc };
	Gpu::Info_lima       _worker_info    { };

	struct Notifier : Syncobj_notifier
	{
		void _handle_notifier()
		{
			lx_emul_submit_completion_signal();
		}

		Genode::Signal_handler<Notifier> _notifier_sigh;

		Notifier(Genode::Entrypoint &ep)
		:
			_notifier_sigh { ep, *this, &Notifier::_handle_notifier }
		{ }

		void notify() override
		{
			_notifier_sigh.local_submit();
		}
	};

	Notifier _sync_notifier { _env.ep() };

	void handle_io_progress() override { }

	Main(Env &env) : _env { env }
	{
		log("--- Lima GPU driver started ---");

		Lx_kit::initialize(_env);
		_env.exec_static_constructors();

		/*
		 * For the moment there is only one task that handles
		 * all sessions, the GPU device is only opened once and
		 * the Buffer registry is global. Therefor only one client
		 * at a time is allowed to access the driver.
		 */
		_worker_args.rm               = &_worker_rm;
		_worker_args.buffers          = &_worker_buffers;
		_worker_args.info             = &_worker_info;
		_worker_args.syncobj_notifier = &_sync_notifier;

		lx_user_task_args    = &_worker_args;

		lx_emul_start_kernel(_dtb_rom.local_addr<void>());

		lx_emul_announce_gpu_session();

		_env.ep().register_io_progress_handler(*this);
	}
};


void Component::construct(Genode::Env &env)
{
	static Driver::Main main(env);
}
