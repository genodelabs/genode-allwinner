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

	using Root_component = Genode::Root_component<Session_component, Genode::Multiple_clients>;

	struct Operation;
	struct Request;
	struct Local_request;

	struct Worker_args;

	struct Ctx_id;
	struct Syncobj_id;


} /* namespace Gpu */


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
		INVALID         = 0,
		ALLOC           = 1,
		FREE            = 2,
		EXEC            = 5,
		WAIT            = 6,
		CTX_CREATE      = 7,
		CTX_FREE        = 8,
		SYNCOBJ_CREATE  = 9,
		SYNCOBJ_DESTROY = 10,
		SYNCOBJ_WAIT    = 11,
		CLOSE           = 14,
		OPEN            = 15,
		FLINK           = 16,
	};

	Type type;

	uint32_t         size;
	uint32_t         va;
	Vram_id          id;
	Sequence_number  seqno;

	uint32_t handle;

	uint32_t name;

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
		case Type::CLOSE:           return "CLOSE";
		case Type::OPEN:            return "OPEN";
		case Type::FLINK:           return "FLINK";
		}
		return "INVALID";
	}

	void print(Genode::Output &out) const
	{
		Genode::print(out, type_name(type));
		if (type == Type::CTX_CREATE || type == Type::CTX_FREE || type == Type::EXEC)
			Genode::print(out, " ctx_id: ", ctx_id.value);
	}
};


struct Gpu::Request
{
	void const *session;

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
		Genode::print(out, "session=", session, " "
		                   "tag=",       tag.value, " "
		                   "success=",   success,   " "
		                   "operation=", operation);
	}

	static Gpu::Request create(void *session, Operation::Type type)
	{
		static unsigned long tag_counter = 0;

		return Gpu::Request {
			.session = session,
			.operation = Operation {
				.type = type,
				.size = 0,
				.va = 0,
				.id = Vram_id { .value = 0 },
				.seqno = Sequence_number { .value = 0 },
				.handle = 0,
				.name = 0,
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


struct Buffer_object
{
	Genode::Id_space<Buffer_object>::Element const _elem;

	Genode::uint32_t             const handle;
	Genode::Dataspace_capability const cap;
	Genode::Attached_dataspace         attached_ds;

	Genode::uint32_t const va;

	Buffer_object(Genode::Id_space<Buffer_object> &space,
	              Gpu::Vram_id                     id,
	              Genode::uint32_t                 handle,
	              Genode::uint32_t                 va,
	              Genode::Dataspace_capability     cap,
	              Genode::Region_map              &rm)
	:
		_elem       { *this, space,
		              Genode::Id_space<Buffer_object>::Id { .value = id.value } },
		handle      { handle },
		cap         { cap },
		attached_ds { rm, cap },
		va          { va }
	{ }
};


extern "C" void lx_emul_mem_cache_clean_invalidate(const void * addr,
                                                   unsigned long size);

struct Buffer_object_space : Genode::Id_space<Buffer_object>
{
	Genode::Allocator &_alloc;

	Buffer_object_space(Genode::Allocator &alloc) : _alloc { alloc } { }

	~Buffer_object_space()
	{ }

	template <typename FN>
	void _try_apply(Gpu::Vram_id id, FN const &fn)
	{
		try {
			Genode::Id_space<Buffer_object>::Id buffer_id { .value = id.value };
			apply<Buffer_object>(buffer_id, fn);
		} catch (Buffer_object_space::Unknown_id) { }
	}

	void *local_addr(Gpu::Vram_id id)
	{
		void *local_addr = nullptr;
		_try_apply(id, [&] (Buffer_object &b) {
			local_addr = b.attached_ds.local_addr<void>();
		});
		return local_addr;
	}

	struct Lx_handle
	{
		Genode::uint32_t value;

		bool _valid;

		bool valid() const { return _valid; }
	};

	Lx_handle lookup_and_flush(Gpu::Vram_id id, bool flush)
	{
		Lx_handle result { 0, false };

		_try_apply(id, [&] (Buffer_object &b) {
				if (flush)
					lx_emul_mem_cache_clean_invalidate(b.attached_ds.local_addr<void>(),
					                                   b.attached_ds.size());
				result = { b.handle, true };
		});

		return result;
	}

	void insert(Gpu::Vram_id id, Genode::uint32_t handle, Genode::uint32_t va,
	            Genode::Dataspace_capability cap, Genode::Region_map &rm)
	{
		// XXX assert id is not assosicated with other handle and
		//     handle is not already present in registry
		new (&_alloc) Buffer_object(*this, id, handle, va, cap, rm);
	}

	void remove(Gpu::Vram_id id)
	{
		bool removed = false;
		_try_apply(id, [&] (Buffer_object &b) {
			Genode::destroy(_alloc, &b);
			removed = true;
		});

		if (!removed)
			Genode::warning("could not remove buffer with id: ", id.value,
			                " - not present in registry");
	}

	Genode::Dataspace_capability lookup_buffer(Gpu::Vram_id id)
	{
		Genode::Dataspace_capability cap { };
		_try_apply(id, [&] (Buffer_object const &b) {
			cap = b.cap;
		});
		return cap;
	}

	template <typename FN>
	void with_handle(Gpu::Vram_id id, FN const &fn)
	{
		_try_apply(id, [&] (Buffer_object const &b) {
			fn(b.handle);
		});
	}

	template <typename FN>
	void with_bo(Gpu::Vram_id id, FN const &fn)
	{
		_try_apply(id, [&] (Buffer_object &b) {
			fn(b);
		});
	}

	bool managed(Gpu::Vram_id id)
	{
		bool result = false;
		_try_apply(id, [&] (Buffer_object const &) {
				result = true;
		});
		return result;
	}
};


struct Gpu::Vram : Genode::Interface
{
	GENODE_RPC_INTERFACE();
};


struct Vram_owner
{
	Genode::Capability<Gpu::Session> const cap;

	Vram_owner(Genode::Capability<Gpu::Session> cap)
	: cap { cap } { }
};


struct Gpu_vram : Genode::Rpc_object<Gpu::Vram>
{
	Buffer_object const &bo;
	Vram_owner    const &_owner;

	struct Import_name
	{
		Genode::uint32_t value;

		bool _valid;

		bool valid() const { return _valid; }
	};

	Import_name import_name;

	Gpu_vram(Buffer_object const &bo,
	         Vram_owner    const &owner)
	:
		bo { bo },
		_owner { owner },
		import_name { 0, false }
	{ }

	bool owner(Genode::Capability<Gpu::Session> other) const
	{
		return _owner.cap == other;
	}
};


struct Vram_local
{
	Genode::Id_space<Vram_local>::Element const _elem;

	Gpu::Vram_capability vram_cap;

	struct Import_handle
	{
		Genode::uint32_t  value;
		bool             _valid;

		bool valid() const { return _valid; }
	};

	struct Export_name
	{
		Genode::uint32_t value;
		bool _valid;

		bool valid() const { return _valid; }
	};

	Export_name   export_name   { 0, false };
	Import_handle import_handle { 0, false };

	Vram_local(Genode::Id_space<Vram_local> &space,
	           Gpu::Vram_capability          vram_cap,
	           Gpu::Vram_id                  vram_id)
	:
		_elem { *this, space,
		        Genode::Id_space<Vram_local>::Id { .value = vram_id.value } },
		vram_cap { vram_cap }
	{ }
};


struct Vram_local_space : Genode::Id_space<Vram_local>
{
	Genode::Entrypoint &_ep;
	Genode::Allocator  &_alloc;

	template <typename FN>
	void _try_apply(Gpu::Vram_id id, FN const &fn)
	{
		try {
			Vram_local_space::Id vl_id { .value = id.value };
			apply<Vram_local>(vl_id, fn);
		} catch (Vram_local_space::Unknown_id) { }
	}

	Vram_local_space(Genode::Entrypoint &ep,
	                 Genode::Allocator  &alloc)
	: _ep { ep }, _alloc { alloc } { }

	~Vram_local_space()
	{ }

	void remove(Gpu::Vram_id id)
	{
		bool removed = false;
		_try_apply(id, [&] (Vram_local &vl) {
			Genode::destroy(_alloc, &vl);
			removed = true;
		});

		if (!removed)
			Genode::warning("could not remove vram local with id: ", id.value,
			                " - not present in space");
	}

	Gpu::Vram_capability lookup_vram_cap(Gpu::Vram_id id)
	{
		Gpu::Vram_capability cap { };
		_try_apply(id, [&] (Vram_local const &vl) {
			cap = vl.vram_cap;
		});
		return cap;
	}

	Vram_local::Export_name lookup_export(Gpu::Vram_id id)
	{
		Vram_local::Export_name export_name { 0, false };
		_try_apply(id, [&] (Vram_local const &vl) {
			export_name = vl.export_name;
		});
		return export_name;
	}

	Vram_local::Import_handle lookup_import(Gpu::Vram_id id)
	{
		Vram_local::Import_handle import_handle { 0, false };

		_try_apply(id, [&] (Vram_local const &vl) {
			import_handle = vl.import_handle;
		});
		return import_handle;
	}

	// XXX make const
	bool contains(Gpu::Vram_id id)
	{
		bool found = false;
		_try_apply(id, [&] (Vram_local &) {
			found = true;
		});
		return found;
	}

	template <typename FN>
	void with_vram_local(Gpu::Vram_id id, FN const &fn)
	{
		_try_apply(id, fn);
	}

	template <typename FN>
	void with_bo(Gpu::Vram_id id, FN const &fn)
	{
		_try_apply(id, [&] (Vram_local &vl) {
			_ep.rpc_ep().apply(vl.vram_cap, [&] (Gpu_vram *v) {
				fn(v->bo);
			});
		});
	}
};


struct Syncobj_notifier : Genode::Interface
{
	virtual void notify() = 0;
};


struct Gpu::Worker_args
{
	Region_map   &rm;
	Buffer_object_space &buffers;
	Vram_local_space    &vram_local_space;

	Syncobj_notifier &_syncobj_notifier;

	struct task_struct *_gpu_task { nullptr };

	Gpu::Request       *_pending_request   { nullptr };
	Gpu::Request       *_completed_request { nullptr };
	Gpu::Local_request *_local_request     { nullptr };

	void *drm { nullptr };

	Gpu::Info_lima info { };

	Worker_args(Region_map &rm, Buffer_object_space &buffers,
	            Vram_local_space &vram_local_space,
	            Syncobj_notifier &notifier)
	:
		rm { rm }, buffers { buffers }, vram_local_space { vram_local_space },
		_syncobj_notifier { notifier }
	{ }

	void signal_syncobj_wait(void)
	{
		_syncobj_notifier.notify();
	}

	template <typename FN> void with_local_request(FN const &fn)
	{
		if (!_local_request)
			return;

		fn(*_local_request);
	}

	template <typename FN> void with_pending_request(FN const &fn)
	{
		if (!_pending_request || !_pending_request->valid() || !_completed_request)
			return;

		/*
 		 * Reset first to prevent _schedule_request from picking up
		 * unfinished requests, e.g. SYNCOBJ_WAIT.
		 */
		*_completed_request = Gpu::Request();

		*_completed_request = fn(*_pending_request);
		*_pending_request   = Gpu::Request();
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


/* implemented in 'lx_user.c' */
extern "C" struct task_struct *lx_user_task;
extern "C" void               *lx_user_task_args;
extern "C" struct task_struct *lx_user_new_gpu_task(int (*func)(void*), void *args);
extern "C" void                lx_user_destroy_gpu_task(struct task_struct*);


/*
 * Function executed by each worker task that handles all
 * request dispatching into the kernel.
 */
extern "C" int gpu_task_func(void *p)
{
	Gpu::Worker_args &args = *static_cast<Gpu::Worker_args*>(p);

	using namespace Genode;
	using OP = Gpu::Operation::Type;

	while (true) {

		/*
		 * 'buffers' is used for BO management (alloc/free, wait
		 * and export) is only called by the primary session.
		 */
		Buffer_object_space &buffers          = args.buffers;
		Region_map          &rm               = args.rm;

		/*
		 * 'vram_local_space' is used for submitting and only used
		 * by secondary sessions.
		 */
		Vram_local_space &vram_local_space = args.vram_local_space;

		/* handle local requests first */
		bool destroy_task = false;

		args.with_local_request([&] (Gpu::Local_request &lr) {
			lr.success = false;
			switch (lr.type) {
			case Gpu::Local_request::Type::OPEN:

				if (!args.drm) {
					args.drm = lx_drm_open();
					if (args.drm) {
						_populate_info(args.drm, args.info);

						lr.success = true;
					}
				}
				break;
			case Gpu::Local_request::Type::CLOSE:
				if (args.drm) {
					lx_drm_close(args.drm);
					args.drm = nullptr;
					destroy_task = true;

					lr.success = true;
				}
				break;
			case Gpu::Local_request::Type::INVALID:
				break;
			}
		});

		if (destroy_task)
			break;

		/* handle pending request */
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
					lx_drm_gem_close(args.drm, handle);
					break;
				}

				if (va != va_allocated) {
					error("va wrong ", Hex(va), " != ", Hex(va_allocated));
					lx_drm_gem_close(args.drm, handle);
					break;
				}

				Dataspace_capability cap =
					genode_lookup_cap(args.drm, offset, size);
				buffers.insert(r.operation.id, handle, va, cap, rm);

				r.success = true;
				break;
			}
			case OP::CLOSE:
			{
				int const err = lx_drm_gem_close(args.drm, r.operation.handle);
				if (err) {
					error("lx_drm_gem_close failed: ", err);
					break;
				}

				r.success = true;
				break;
			}
			case OP::OPEN:
			{
				uint32_t handle;
				uint64_t size;

				int const err = lx_drm_gem_open(args.drm, r.operation.name,
				                                &handle, &size);
				if (err) {
					error("lx_drm_gem_open failed: ", err);
					break;
				}

				r.operation.handle = handle;
				r.operation.size   = (uint32_t)size;

				r.success = true;
				break;
			}
			case OP::FLINK:
			{
				uint32_t name;

				buffers.with_handle(r.operation.id, [&] (uint32_t const handle) {
					int const err = lx_drm_gem_flink(args.drm, handle, &name);
					if (err) {
						error("lx_drm_gem_flink failed: ", err);
						return;
					}

					r.operation.name = name;

					r.success = true;
				});
				break;
			}
			case OP::FREE:
			{
				bool bo_found = false;
				buffers.with_handle(r.operation.id, [&] (uint32_t const handle) {
					(void)lx_drm_gem_close(args.drm, handle);
					bo_found = true;
				});

				if (bo_found) {
					buffers.remove(r.operation.id);
					r.success = true;
				}
				break;
			}
			case OP::EXEC:
			{
				void *gem_submit = nullptr;
				vram_local_space.with_bo(r.operation.id, [&] (Buffer_object const &bo) {
					gem_submit = const_cast<void*>(bo.attached_ds.local_addr<void>());
				});
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
					Gpu::Vram_id id { .value = *bo_handle };
					/* flush only when read by the GPU */
					vram_local_space.with_bo(id, [&] (Buffer_object const &bo) {
						if (bo_read)
							lx_emul_mem_cache_clean_invalidate(bo.attached_ds.local_addr<void>(),
							                                   bo.attached_ds.size());
					});
					Vram_local::Import_handle const handle = vram_local_space.lookup_import(id);
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

		args.with_pending_request(dispatch_pending);

		if (notify_client)
			args.signal_syncobj_wait();

		lx_emul_task_schedule(true);
	}

	lx_user_destroy_gpu_task(args._gpu_task);
	return 0;
}


/**
 * Function executed by the the 'lx_user' task solely used to
 * create new GPU worker tasks.
 */
struct Lx_user_task_args
{
	bool create_task;
	void *args;

	struct task_struct *new_gpu_task;
};
static Lx_user_task_args _lx_user_task_args { };


extern "C" int lx_user_task_func(void *p)
{
	Lx_user_task_args &args = *static_cast<Lx_user_task_args*>(p);

	while (true) {
		if (args.create_task) {
			args.new_gpu_task = lx_user_new_gpu_task(gpu_task_func, args.args);
			args.create_task = false;
		}

		lx_emul_task_schedule(true);
	}
}


/*
 * Helper utility that will create a new Gpu worker task
 * be executing the 'lx_user' so that those are created
 * from within the kernel context.
 */
static struct task_struct *create_gpu_task(void *args)
{
	if (_lx_user_task_args.create_task)
		return nullptr;

	_lx_user_task_args.args        = args;
	_lx_user_task_args.create_task = true;

	lx_emul_task_unblock(lx_user_task);
	Lx_kit::env().scheduler.schedule();

	return _lx_user_task_args.new_gpu_task;
}


struct Gpu::Session_component : public Genode::Session_object<Gpu::Session>
{
	private:

		Session_component(Session_component const&) = delete;
		Session_component& operator=(Session_component const&) = delete;

		Genode::Env        &_env;
		Genode::Entrypoint &_ep;

		Vram_owner _owner { cap() };

		Genode::Heap         _heap   { _env.ram(), _env.rm() };
		Buffer_object_space  _worker_buffers { _heap };

		Vram_local_space _vram_local_space { _ep, _heap };

		Genode::Id_space<Session_component>::Element const _elem;

		Genode::Attached_ram_dataspace _info_dataspace {
			_env.ram(), _env.rm(), 4096 };
		Genode::Signal_context_capability _completion_sigh { };

		Gpu::Request _pending_request   { };
		Gpu::Request _completed_request { };

		Gpu::Worker_args  _lx_task_args;
		task_struct      *_lx_task;

		bool _managed_id(Gpu::Request const &request)
		{
			using OP = Gpu::Operation::Type;

			switch (request.operation.type) {
			case OP::FREE:  [[fallthrough]];
			case OP::EXEC:
				return _lx_task_args.buffers.managed(request.operation.id);
			default:
				break;
			}

			return true;
		}

		enum class Blocking_request : uint8_t { NO, YES };

		/*
		 * Most request will block the RPC interface as the Gpu session
		 * requires this behaviour. 'WAIT' and 'SYNCOBJ_WAIT' are the
		 * only RPCs performed in polling fashion.
		 */
		template <typename SUCC_FN, typename FAIL_FN>
		void _schedule_request(Gpu::Request const &request,
		                       SUCC_FN const &succ_fn,
		                       FAIL_FN const &fail_fn,
		                       Blocking_request block = Blocking_request::YES)
		{
			if (_pending_request.valid()) {
				/* that should not happen and is most likely a bug in the client */
				error(__func__, ": ", this, ": request pending: ", _pending_request,
				      " cannot schedule new request: ", request);
				fail_fn();
				return;
			}

			/*
			 * Requests referencing not managed handles will be
			 * treated as scheduled but failed.
			 */
			if (!_managed_id(request)) {
				fail_fn();
				return;
			}

			_pending_request = request;
			_lx_task_args._pending_request   = &_pending_request;
			_lx_task_args._completed_request = &_completed_request;

			lx_emul_task_unblock(_lx_task);
			Lx_kit::env().scheduler.schedule();

			if (block == Blocking_request::YES)
				for (;;) {
					if (_completed_request.matches(request)
					 && _completed_request.valid())
						break;

					_ep.wait_and_dispatch_one_io_signal();
				}

			if (_completed_request.success) {
				succ_fn(_completed_request);
			}
			else {
				fail_fn();
			}
		}

		bool _local_request(Gpu::Local_request::Type type)
		{
			Gpu::Local_request local_request {
				.type = type,
				.success = false,
			};
			_lx_task_args._local_request = &local_request;

			// XXX must not return prematurely
			lx_emul_task_unblock(_lx_task);
			Lx_kit::env().scheduler.schedule();

			bool const success = _lx_task_args._local_request->success;
			_lx_task_args._local_request = nullptr;

			return success;
		}

		Gpu::Ctx_id _ctx_id { 0 };

		Gpu::Ctx_id _create_ctx()
		{
			Gpu::Ctx_id ctx_id { 0 };

			Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::CTX_CREATE);

			auto success = [&] (Gpu::Request const &request) {
				ctx_id = request.operation.ctx_id;
			};
			auto fail = [&] () { };
			_schedule_request(r, success, fail);

			return ctx_id;
		}

		void _free_ctx(Gpu::Ctx_id ctx_id)
		{
			Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::CTX_FREE);
			r.operation.ctx_id = ctx_id;

			auto success = [&] (Gpu::Request const &) { };
			auto fail = [&] () { warning("could not free ctx_id: ", ctx_id.value); };
			_schedule_request(r, success, fail);
		}

		Gpu::Syncobj_id _sync_id[Gpu::Operation::MAX_PIPE] { { 0 }, { 0 } };

		Gpu::Syncobj_id _create_syncobj()
		{
			Gpu::Syncobj_id syncobj_id { 0 };

			Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::SYNCOBJ_CREATE);

			auto success = [&] (Gpu::Request const &request) {
				syncobj_id = request.operation.syncobj_id[0];
			};
			auto fail = [&] () { };
			_schedule_request(r, success, fail);

			return syncobj_id;
		}

		void _destroy_syncobj(Gpu::Syncobj_id syncobj_id)
		{
			Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::SYNCOBJ_DESTROY);
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

		struct Notifier : Syncobj_notifier
		{
			void _handle_notifier()
			{
				_sc.submit_completion_signal();
			}

			Genode::Signal_handler<Notifier> _notifier_sigh;
			Session_component &_sc;

			Notifier(Genode::Entrypoint &ep, Session_component &sc)
			:
				_notifier_sigh { ep, *this, &Notifier::_handle_notifier },
				_sc { sc }
			{ }

			void notify() override
			{
				_notifier_sigh.local_submit();
			}
		};

		Notifier _notifier { _env.ep(), *this };

		/*
		 * Free buffer-object
		 *
		 * Implemented here as it is used by 'unmap_gpu' as well as the
		 * Session_component destructor to clean up.
		 */
		bool _unmap_gpu(Vram_id id, Genode::off_t, Virtual_address)
		{
			Gpu::Vram_capability const cap = _vram_local_space.lookup_vram_cap(id);
			if (!cap.valid()) {
				Genode::warning(__func__, ": could not look up Vram_capability for id ", id.value);
				return false;
			}

			// XXX instead of procedural execution nested access

			bool owner = false;
			_env.ep().rpc_ep().apply(cap, [&] (Gpu_vram *v) {
				owner = v->owner(_owner.cap);
			});

			if (!owner) {
				Genode::warning(__func__, ": cannot free id: ", id.value, " not the owner");
				return false;
			}

			Gpu_vram *vp = nullptr;
			_env.ep().rpc_ep().apply(cap, [&] (Gpu_vram *v) {
				_env.ep().dissolve(*v);
				vp = v;
			});

			Vram_local *vlp = nullptr;
			_vram_local_space.with_vram_local(id, [&] (Vram_local &vl) {
				vlp = &vl;
			});

			Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::FREE);
			r.operation.id = id;

			if (vlp) {
				Vram_local::Export_name const export_name = vlp->export_name;
				if (export_name.valid()) {
					r.operation.handle = export_name.value;
				}

				Genode::destroy(_heap, vlp);
			}

			Genode::destroy(_heap, vp);

			auto success = [&] (Gpu::Request const &) { };
			auto fail = [&] () { };
			_schedule_request(r, success, fail);

			return true;
		}

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
		                  Genode::Id_space<Session_component> &space)
		:
			Session_object { ep, resources, label, diag },
			_env           { env },
			_ep            { ep },
			_elem          { *this, space },
			_lx_task_args  { _env.rm(), _worker_buffers, _vram_local_space, _notifier },
			_lx_task       { create_gpu_task(&_lx_task_args ) }
		{
			if (!_lx_task) {
				Genode::error("could not create GPU task");
				throw Could_not_open_drm();
			}

			_lx_task_args._gpu_task = _lx_task;

			if (!_local_request(Gpu::Local_request::Type::OPEN)) {
				Genode::warning("could not open DRM session");
				throw Could_not_open_drm();
			}

			_ctx_id = _create_ctx();

			_sync_id[0] = _create_syncobj();
			_sync_id[1] = _create_syncobj();

			void *info = _info_dataspace.local_addr<void>();
			Genode::memcpy(info, &_lx_task_args.info, sizeof (Gpu::Info_lima));
		}

		virtual ~Session_component()
		{
			/*
			 * Close all imported handles (those were not created by this
			 * sessions so the the export handle must be invalid).
			 */
			_vram_local_space.for_each<Vram_local>([&] (Vram_local &vl) {

				if (vl.export_name.valid())
					return;

				if (!vl.import_handle.valid())
					return;

				Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::CLOSE);
				r.operation.id = Gpu::Vram_id { .value = vl._elem.id().value };
				r.operation.handle = vl.import_handle.value;

				auto success = [&] (Gpu::Request const &) { };
				auto fail = [&] () { };
				_schedule_request(r, success, fail);
			});

			/*
			 * Close all exported handles and free buffer objects owned by this session.
			 *
			 * Additionally destroy all imported handles we closed before.
			 */
			while (_vram_local_space.apply_any<Vram_local>([&] (Vram_local &vl) {

				if (vl.import_handle.valid()) {
					Genode::destroy(_heap, &vl);
					return;
				}

				if (!_unmap_gpu(Gpu::Vram_id { .value = vl._elem.id().value }, 0, Gpu::Virtual_address { 0 }))
					Genode::destroy(_heap, &vl);
			})) { ; }

			if (_lx_task_args._pending_request->valid()) {
				Genode::warning("destructor override currently pending request");
			}

			for (unsigned i = 0; i < Gpu::Operation::MAX_PIPE; i++) {
				_destroy_syncobj(_sync_id[i]);
			}

			_free_ctx(_ctx_id);

			if (!_local_request(Gpu::Local_request::Type::CLOSE))
				Genode::warning("could not close DRM session - leaking objects");
		}

		void submit_completion_signal()
		{
			if (_completion_sigh.valid())
				Genode::Signal_transmitter(_completion_sigh).submit();
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
			Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::EXEC);
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
			/* ignore any seqno we are not aware of */
			bool valid = false;
			for (auto v : _sync_id) {
				if (seqno.value == v.value) {
					valid = true;
					break;
				}
			}
			if (!valid)
				return false;

			/* handle previous interrupted call */
			if (_pending_syncobj_wait) {
				Gpu::Request *pr = _lx_task_args._pending_request;
				if (pr && pr->operation.type == Gpu::Operation::Type::SYNCOBJ_WAIT) {
					return false;
				}

				Gpu::Request *cr = _lx_task_args._completed_request;
				if (cr && cr->operation.type == Gpu::Operation::Type::SYNCOBJ_WAIT) {
					_pending_syncobj_wait = false;
					return cr->success;
				}
			}

			Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::SYNCOBJ_WAIT);
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
			return _worker_buffers.lookup_buffer(id);
		}

		void unmap_cpu(Vram_id id) override
		{
			Vram_local *vlp = nullptr;
			_vram_local_space.with_vram_local(id, [&] (Vram_local &vl) {

				/* it should be safe to destroy the vl in all cases */
				vlp = &vl;

				// XXX check that is does not deferr BO freeing
				if (vl.export_name.valid())
					return;

				if (!vl.import_handle.valid())
					return;

				Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::CLOSE);
				r.operation.id = Gpu::Vram_id { .value = vl._elem.id().value };
				r.operation.handle = vl.import_handle.value;

				auto success = [&] (Gpu::Request const &) { };
				auto fail = [&] () { };
				_schedule_request(r, success, fail);
			});

			if (vlp)
				Genode::destroy(_heap, vlp);
		}

		bool map_gpu(Vram_id id, Genode::size_t size, Genode::off_t,
		             Virtual_address va) override
		{
			Genode::Dataspace_capability cap { };

			cap = _worker_buffers.lookup_buffer(id);
			if (cap.valid()) {
				error("Duplicate 'map_gpu' called for ", id.value);
				return false;
			}

			if (size > ~0U) {
				error("Allocation of buffers > 4G not supported!");
				return false;
			}

			Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::ALLOC);
			r.operation.id   = id;
			r.operation.va   = (uint32_t) va.value;
			r.operation.size = (uint32_t) size;

			bool ret = false;
			auto success = [&] (Gpu::Request const &) {
				ret = true;
			};
			auto fail = [&] () { };
			_schedule_request(r, success, fail);

			if (ret) {
				/*
				 * When we cannot create our book-keeping objects we have to
				 * rollback the BO allocation.
				 */
				ret = false;
				_worker_buffers.with_bo(id, [&] (Buffer_object const &bo) {
					Gpu_vram *vram = nullptr;

					try {
						vram = new (_heap) Gpu_vram(bo, _owner);
					} catch (...) {
						return;
					}

					// XXX can it throw?
					_env.ep().manage(*vram);

					try {
						new (_heap) Vram_local(_vram_local_space, vram->cap(), id);
					} catch (...) {
						_env.ep().dissolve(*vram);
						Genode::destroy(_heap, vram);
						return;
					}

					ret = true;
				});
				if (!ret) {
					Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::FREE);
					r.operation.id   = id;

					auto success = [&] (Gpu::Request const &) { };
					auto fail = [&] () {
						error("map_gpu: ", this, " failed to clean up vram: ", id.value,
						      " might leak: ", size, " bytes of memory");
					};
					_schedule_request(r, success, fail);
				}
			}
			return ret;
		}

		void unmap_gpu(Vram_id id, Genode::off_t, Virtual_address) override
		{
			(void)_unmap_gpu(id, 0, Gpu::Virtual_address { 0 });
		}

		bool set_tiling_gpu(Gpu::Vram_id id, Genode::off_t,  unsigned mode) override
		{
			/* handle previous interrupted call */
			if (_pending_wait) {
				Gpu::Request *pr = _lx_task_args._pending_request;
				if (pr && pr->operation.type == Gpu::Operation::Type::WAIT) {
					return false;
				}

				Gpu::Request *cr = _lx_task_args._completed_request;
				if (cr && cr->operation.type == Gpu::Operation::Type::WAIT) {
					_pending_wait = false;
					return cr->success;
				}
			}

			Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::WAIT);
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

		Gpu::Vram_capability export_vram(Gpu::Vram_id id) override
		{
			if (!_vram_local_space.contains(id))
				return Gpu::Vram_capability();

			Vram_local::Export_name export_name =
				_vram_local_space.lookup_export(id);

			/*
			 * Attempt to export first in case the buffer has not
			 * been exported before and look up capability afterwards.
			 * Also store the global handle for later use on import.
			 */

			if (!export_name.valid()) {

				_vram_local_space.with_vram_local(id, [&] (Vram_local &vl) {
					Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::FLINK);
					r.operation.id = id;

					auto success = [&] (Gpu::Request const &request) {
						/* store for second check below */
						export_name =
							Vram_local::Export_name { request.operation.name, true };

						/* set export handle for future use */
						vl.export_name = export_name;
					};
					auto fail = [&] () { };
					_schedule_request(r, success, fail);
				});
			}

			if (export_name.valid()) {
				Gpu::Vram_capability cap { };

				cap = _vram_local_space.lookup_vram_cap(id);
				_env.ep().rpc_ep().apply(cap, [&] (Gpu_vram *v) {

					if (v->import_name.valid())
						return;

					/* set global name used for import later on */
					v->import_name =
						Gpu_vram::Import_name { .value = export_name.value, true };
				});
				return cap;
			}

			return Gpu::Vram_capability();
		}

		void import_vram(Gpu::Vram_capability cap, Gpu::Vram_id id) override
		{
			if (!_vram_local_space.contains(id))
				try {
					new (_heap) Vram_local(_vram_local_space, cap, id);
				} catch (...) {
					throw Gpu::Session::Out_of_ram();
				}

			if (_vram_local_space.lookup_import(id).valid())
				return;

			_env.ep().rpc_ep().apply(cap, [&] (Gpu_vram *v) {

				Gpu::Request r = Gpu::Request::create(this, Gpu::Operation::Type::OPEN);
				r.operation.name = v->import_name.value;

				bool result = false;
				_vram_local_space.with_vram_local(id, [&] (Vram_local &vl) {

					auto success = [&] (Gpu::Request const &request) {
						result = request.success;
						vl.import_handle =
							Vram_local::Import_handle { request.operation.handle,
							                            true };
					};
					auto fail = [&] () { };
					_schedule_request(r, success, fail);
				});

				if (!result)
					throw Invalid_state();
			});
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

		Gpu::Session_space _session_space { };


	protected:

		Session_component *_create_session(char const *args) override
		{
			Session::Label const label  { session_label_from_args(args) };

			Session_component *sc =
				new (_alloc) Session_component(_env, _env.ep(),
			                                   session_resources_from_args(args),
			                                   label,
			                                   session_diag_from_args(args),
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
			Genode::destroy(md_alloc(), sc);
		}

	public:

		Root(Genode::Env &env, Genode::Allocator &alloc)
		:
			Root_component { env.ep(), alloc },
			_env           { env },
			_alloc         { alloc }
		{ }
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

	void handle_io_progress() override { }

	Main(Env &env) : _env { env }
	{
		log("--- Lima GPU driver started ---");

		Lx_kit::initialize(_env);
		_env.exec_static_constructors();

		_lx_user_task_args.create_task  = false;
		_lx_user_task_args.args         = nullptr;
		_lx_user_task_args.new_gpu_task = nullptr;
		lx_user_task_args = &_lx_user_task_args;

		lx_emul_start_kernel(_dtb_rom.local_addr<void>());

		lx_emul_announce_gpu_session();

		_env.ep().register_io_progress_handler(*this);
	}
};


void Component::construct(Genode::Env &env)
{
	static Driver::Main main(env);
}
