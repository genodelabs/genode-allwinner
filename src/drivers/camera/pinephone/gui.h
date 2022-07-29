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

#ifndef _INCLUDE__GENODE_C_API__GUI_H_
#define _INCLUDE__GENODE_C_API__GUI_H_

/* Genode includes */
#include <genode_c_api/base.h>

#ifdef __cplusplus
extern "C" {
#endif

void genode_gui_init(struct genode_env *env_ptr,
                     struct genode_allocator *alloc_ptr);

struct genode_gui;  /* definition is private to the implementation */


/**************************
 ** Gui-session lifetime **
 **************************/

struct genode_gui_args
{
	char const *label;
	unsigned    width;
	unsigned    height;
};

struct genode_gui *genode_gui_create(struct genode_gui_args const *);

void genode_gui_destroy(struct genode_gui *);


/**************************
 ** Gui-session handling **
 **************************/

struct genode_gui_refresh_context;

typedef void (*genode_gui_refresh_content_t)
	(struct genode_gui_refresh_context *, unsigned char *fb,
	 unsigned long fb_size);

void genode_gui_refresh(struct genode_gui *,
                        genode_gui_refresh_content_t,
                        struct genode_gui_refresh_context *);


#ifdef __cplusplus
}
#endif

#endif /* _INCLUDE__GENODE_C_API__GUI_H_ */
