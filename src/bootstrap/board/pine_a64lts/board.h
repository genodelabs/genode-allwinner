/*
 * \brief  Board driver for bootstrap
 * \author Norman Feske
 * \date   2021-01-21
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _BOOTSTRAP__SPEC__PINE_A64LTS__BOARD_H_
#define _BOOTSTRAP__SPEC__PINE_A64LTS__BOARD_H_

#include <hw/spec/arm_64/pine_a64lts_board.h>
#include <hw/spec/arm_64/cpu.h>
#include <hw/spec/arm/gicv2.h>
#include <hw/spec/arm/lpae.h>

namespace Board {

	using namespace Hw::Pine_a64lts_board;

	struct Cpu : Hw::Arm_64_cpu
	{
		static void wake_up_all_cpus(void*);
	};

	using Pic = Hw::Gicv2;

	Hw::size_t detect_ram_size();

	static constexpr bool NON_SECURE = true;
};

#endif /* _BOOTSTRAP__SPEC__PINE_A64LTS__BOARD_H_ */
