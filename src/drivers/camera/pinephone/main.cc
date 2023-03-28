/*
 * \brief  PinePhone camera driver port
 * \author Josef Soentgen
 * \date   2022-07-29
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

/* Genode includes */
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <base/env.h>

#include <lx_emul/init.h>
#include <lx_emul/shared_dma_buffer.h>
#include <lx_kit/env.h>
#include <lx_kit/init.h>
#include <lx_user/io.h>

#include "lx_user.h"
#include "gui.h"

using namespace Genode;

/* initialized with default values by lx_user */
extern struct lx_user_config_t *lx_user_config;


static unsigned check_and_constrain_value(Xml_node const &node,
                                          char const *attr,
                                          unsigned min, unsigned max)
{
	unsigned value = node.attribute_value(attr, min);
	if (value < min) {
		warning(attr, " limit to ", min);
		value = min;
	}
	else if (value > max) {
		warning(attr, " limit to ", max);
		value = max;
	}

	return value;
}


struct Main
{
	Env                   &env;
	Attached_rom_dataspace dtb_rom        { env, "dtb"           };
	Signal_handler<Main>   signal_handler { env.ep(), *this,
	                                        &Main::handle_signal };
	Sliced_heap            sliced_heap    { env.ram(), env.rm()  };

	Attached_rom_dataspace config_rom     { env, "config"        };

	void _update_config()
	{
		if (!config_rom.valid())
			return;

		config_rom.update();
		Xml_node const config = config_rom.xml();

		lx_user_config_t &lx_config = *lx_user_config;

		lx_config.width =
			check_and_constrain_value(config, "width", (unsigned)MIN_WIDTH,
			                                           (unsigned)MAX_WIDTH);
		lx_config.height =
			check_and_constrain_value(config, "height", (unsigned)MIN_HEIGHT,
			                                            (unsigned)MAX_HEIGHT);
		lx_config.fps =
			check_and_constrain_value(config, "fps", (unsigned)MIN_FPS,
			                                         (unsigned)MAX_FPS);
		lx_config.num_buffer =
			check_and_constrain_value(config, "num_buffer",
			                          (unsigned)MIN_BUFFER,
			                          (unsigned)MAX_BUFFER);

		lx_config.skip_frames =
			check_and_constrain_value(config, "skip_frames",
			                          0u, lx_config.fps);

		lx_config.convert = config.attribute_value("convert", true);
		lx_config.gray    = config.attribute_value("gray", true);
		lx_config.rotate  = config.attribute_value("rotate", true);

		using Format = String<8>;
		Format format { };
		format = config.attribute_value("format", Format("yuv"));
		if (format == "yuv") lx_config.format = FMT_YUV;
		else warning("invalid format selection, using yuv");

		using Camera = String<16>;
		Camera cam { };
		cam = config.attribute_value("camera", Camera("front"));
		if      (cam == "front") lx_config.camera = CAMERA_FRONT;
		else if (cam == "rear")  lx_config.camera = CAMERA_REAR;
		else warning("invalid camera selection, using front camera");

		lx_config.valid = true;

		log("Use ", cam, " camera configuration: ",
		    lx_config.width, "x", lx_config.height, "@",
		    lx_config.fps, "/", lx_config.skip_frames,
		    " (", format, ")", " rotate: ", lx_config.rotate,
		    " num_buffer: ", lx_config.num_buffer);
	}

	void handle_signal()
	{
		lx_user_handle_io();
		Lx_kit::env().scheduler.schedule();
	}

	Main(Env & env) : env(env)
	{
		Lx_kit::initialize(env);
		env.exec_static_constructors();

		_update_config();

		genode_gui_init(genode_env_ptr(env),
		                genode_allocator_ptr(sliced_heap));

		lx_emul_start_kernel(dtb_rom.local_addr<void>());
	}
};


void Component::construct(Env & env) { static Main main(env); }
