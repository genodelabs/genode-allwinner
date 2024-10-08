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

		Env &_env;

		Framebuffer::Mode const _mode;

		Gui::Connection    _gui;
		Gui::View_id const _view { };

		Constructible<Attached_dataspace>  _fb_ds { };
		unsigned char                     *_fb_ptr { nullptr };

	public:

		genode_gui(Env &env, Session_label const label, Framebuffer::Mode mode)
		:
			_env(env), _mode(mode), _gui(env, label)
		{
			_gui.buffer(_mode);
			_gui.view(_view, { .title = label, .rect = { }, .front = true });

			_fb_ds.construct(_env.rm(), _gui.framebuffer.dataspace());
			_fb_ptr = _fb_ds->local_addr<unsigned char>();
		}

		void refresh(auto const &fn)
		{
			fn(_fb_ptr, _mode.num_bytes());
		}

		void swap_view(auto const &fn)
		{
			using C = Gui::Session::Command;

			genode_gui_view const view = fn();

			Gui::Point const point { view.x, view.y };
			Gui::Area  const area  { view.width, view.height };

			_gui.enqueue<C::Geometry>(_view, Gui::Rect { { 0, 0 }, area });
			_gui.enqueue<C::Offset>(_view, point);
			_gui.execute();

			/*
			 * The explicit refresh here should probably not be needed
			 * as setting the offset should trigger it as well.
			 */
			_gui.framebuffer.refresh({ { view.x, -view.y }, area });
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

	Framebuffer::Mode const mode { .area  = { args->width, args->height },
	                               .alpha = false };

	return new (*_alloc_ptr)
		Registered<genode_gui>(_gui_sessions, *_env_ptr, args->label, mode);
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


void genode_gui_swap_view(struct genode_gui *gui_ptr,
                          genode_gui_swap_view_t view_cb,
                          struct genode_gui_refresh_context *ctx)
{
	gui_ptr->swap_view([&] {
		return view_cb(ctx);
	});
}
