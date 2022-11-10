/*
 * \brief  Board definitions for Pine-A64
 * \author Norman Feske
 * \date   2021-01-21
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SRC__INCLUDE__HW__SPEC__ARM_64__PINE_A64LTS__BOARD_H_
#define _SRC__INCLUDE__HW__SPEC__ARM_64__PINE_A64LTS__BOARD_H_

#include <drivers/uart/ns16550.h>
#include <hw/spec/arm/boot_info.h>

namespace Hw::Pine_a64lts_board {

	using Serial = Genode::Ns16550_uart;

	enum {
		RAM_BASE   = 0x40000000,
		RAM_SIZE   = 0xbe000000,

		UART_BASE  = 0x1c28000,
		UART_SIZE  = 0x1000,
		UART_CLOCK = 0,
	};

	namespace Cpu_mmio {
		enum {
			IRQ_CONTROLLER_DISTR_BASE   = 0x01c81000,
			IRQ_CONTROLLER_DISTR_SIZE   = 0x1000,
			IRQ_CONTROLLER_CPU_BASE     = 0x01c82000,
			IRQ_CONTROLLER_CPU_SIZE     = 0x2000,
			IRQ_CONTROLLER_VT_CTRL_BASE = 0xbbbbb000,
			IRQ_CONTROLLER_VT_CPU_BASE  = 0x31020000,
			IRQ_CONTROLLER_VT_CPU_SIZE  = 0x2000,
		};
	};
};

#endif /* _SRC__INCLUDE__HW__SPEC__ARM_64__PINE_A64LTS__BOARD_H_ */
