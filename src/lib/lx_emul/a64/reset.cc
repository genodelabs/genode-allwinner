/*
 * \brief  Linux kernel / Lx_emul reset pin control
 * \author Stefan Kalkowski
 * \date   2021-05-25
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul/reset.h>
#include <lx_kit/env.h>

void lx_emul_reset_deassert(const char * name)
{
	Genode::String<64> type("reset-pin,", name);
	Lx_kit::env().devices.for_each([&] (Lx_kit::Device & d) {
		if (type == d.compatible()) { d.enable(); } });
}
