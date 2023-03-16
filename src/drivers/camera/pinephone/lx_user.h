/*
 * \brief  Configuration for lx_user task
 * \author Josef Soentgen
 * \date   2022-09-27
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
	MAX_WIDTH  = 1280,
	MAX_HEIGHT = 720,
	MAX_FPS    = 30,

	MIN_WIDTH  = 160,
	MIN_HEIGHT = 120,
	MIN_FPS    = 1,

	FMT_YUV      = 0,
	FMT_SBGRR8   = 1,
	CAMERA_FRONT = 0,
	CAMERA_REAR  = 1,
};

struct lx_user_config_t
{
	unsigned width;
	unsigned height;
	unsigned fps;
	unsigned format;
	unsigned camera;

	unsigned rotate;
	unsigned convert;

	/* set after parsing the configuration */
	unsigned valid;
};

#ifdef __cplusplus
}
#endif
