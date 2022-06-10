/*
 * \brief  Linux emulation backend functions
 * \author Josef Soentgen
 * \date   2021-03-22
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

/* Genode includes */
#include <base/attached_dataspace.h>
#include <base/attached_ram_dataspace.h>
#include <base/registry.h>
#include <dataspace/client.h>
#include <util/list.h>

/* lx emulation includes */
#include <lx_emul/page_virt.h>
#include <lx_kit/env.h>

/* local includes */
#include "emul.h"

using size_t = Genode::size_t;


void *emul_alloc_shmem_file_buffer(unsigned long size)
{
	return (void*) Lx_kit::env().memory.alloc_buffer(size).virt_addr();
}


void emul_free_shmem_file_buffer(void *addr)
{
	Lx_kit::env().memory.free_buffer(addr);
}


/************
 ** memory **
 ************/

unsigned long emul_user_copy(void *to, void const *from, unsigned long n)
{
	Genode::memcpy(to, from, n);
	return 0;
}


/*****************
 ** Gpu session **
 *****************/

extern "C" void *genode_lookup_mapping_from_offset(void *, unsigned long, unsigned long);

Genode::Dataspace_capability genode_lookup_cap(void *drm, unsigned long long offset,
                                               unsigned long size)
{
	void *addr = genode_lookup_mapping_from_offset(drm, offset, size);
	if (!addr) {
		return Genode::Dataspace_capability();
	}

	return Lx_kit::env().memory.attached_dataspace_cap(addr);
}
