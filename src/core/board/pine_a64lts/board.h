/*
 * \brief  Board driver for core
 * \author Norman Feske
 * \date   2021-01-21
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CORE__SPEC__PINE_A64LTS__BOARD_H_
#define _CORE__SPEC__PINE_A64LTS__BOARD_H_

#include <hw/spec/arm_64/pine_a64lts_board.h>
#include <spec/arm/generic_timer.h>
#include <spec/arm/virtualization/gicv2.h>
#include <spec/arm_64/cpu/vm_state_virtualization.h>
#include <spec/arm/virtualization/board.h>

namespace Board {

	using namespace Hw::Pine_a64lts_board;

	enum {
		TIMER_IRQ           = 14 + 16,
		VT_TIMER_IRQ        = 11 + 16,
		VT_MAINTAINANCE_IRQ = 9  + 16,
		VCPU_MAX            = 16
	};
};

#endif /* _CORE__SPEC__PINE_A64LTS__BOARD_H_ */
