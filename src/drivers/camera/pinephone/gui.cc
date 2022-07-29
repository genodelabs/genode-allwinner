/*
 * \brief  Genode C-API for GUI Session
 * \author Josef Soentgen
 * \date   2022-08-05
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */


/* Genode includes */
#include <base/registry.h>
#include <gui_session/connection.h>

/* local includes */
#include "gui.h"


using namespace Genode;

static Env       *_env_ptr;
static Allocator *_alloc_ptr;
static Registry<Registered<genode_gui>> _gui_sessions { };


struct genode_gui : private Noncopyable, private Interface
{
	private:

		genode_gui(const genode_gui &) = delete;
		const genode_gui& operator=(const genode_gui&) = delete;

		Env       &_env;
		Allocator &_alloc;

		Session_label const _session_label;

		Gui::Connection           _gui  { _env, _session_label.string() };
		Gui::Session::View_handle _view { _gui.create_view() };
		Framebuffer::Mode const   _mode;

		Constructible<Attached_dataspace>  _fb_ds { };
		unsigned char                     *_fb_ptr { nullptr };

	public:

		genode_gui(Env &env, Allocator &alloc,
		           Session_label const &session_label,
		           Framebuffer::Mode mode)
		:
			_env           { env },
			_alloc         { alloc },
			_session_label { session_label },
			_mode          { mode }
		{
			_gui.buffer(_mode, false);

			_fb_ds.construct(_env.rm(), _gui.framebuffer()->dataspace());
			_fb_ptr = _fb_ds->local_addr<unsigned char>();

			using C = Gui::Session::Command;
			using namespace Gui;

			_gui.enqueue<C::Geometry>(_view, Gui::Rect(Gui::Point(0, 0),
			                          _mode.area));
			_gui.enqueue<C::To_front>(_view, Gui::Session::View_handle());
			_gui.enqueue<C::Title>(_view, _session_label.string());
			_gui.execute();
		}

		template <typename FN>
		void refresh(FN const &fn)
		{
			size_t const size = _mode.area.w()
			                  * _mode.area.h()
			                  * _mode.bytes_per_pixel();
			fn(_fb_ptr, size);

			_gui.framebuffer()->refresh(0, 0, _mode.area.w(),
			                                  _mode.area.h());
		}
};


void genode_gui_init(struct genode_env       *env_ptr,
                     struct genode_allocator *alloc_ptr)
{
	_env_ptr   = env_ptr;
	_alloc_ptr = alloc_ptr;
}


struct genode_gui *genode_gui_create(struct genode_gui_args const *args)
{
	if (!_env_ptr || !_alloc_ptr) {
		error("genode_gui_create: missing call of genode_gui_init");
		return nullptr;
	}

	Framebuffer::Mode const mode { { args->width, args->height } };

	return new (*_alloc_ptr)
		Registered<genode_gui>(_gui_sessions, *_env_ptr, *_alloc_ptr,
		                       Session_label(args->label), mode);
}


void genode_gui_destroy(struct genode_gui *gui_ptr)
{
	destroy(*_alloc_ptr, gui_ptr);
}


void genode_gui_refresh(struct genode_gui *gui_ptr,
                        genode_gui_refresh_content_t refresh_cb,
                        struct genode_gui_refresh_context *ctx)
{
	gui_ptr->refresh([&] (unsigned char *dst, size_t size) {
		refresh_cb(ctx, dst, size);
	});
}
