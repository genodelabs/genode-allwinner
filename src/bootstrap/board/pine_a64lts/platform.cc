/*
 * \brief   Platform implementation specific for base-hw and Pine-A64
 * \author  Norman Feske
 * \date    2021-01-21
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <platform.h>

Bootstrap::Platform::Board::Board()
:
	early_ram_regions(Memory_region { ::Board::RAM_BASE, ::Board::detect_ram_size() }),
	late_ram_regions(Memory_region { }),
	core_mmio(Memory_region { ::Board::UART_BASE, ::Board::UART_SIZE },
	          Memory_region { ::Board::Cpu_mmio::IRQ_CONTROLLER_DISTR_BASE,
	                          ::Board::Cpu_mmio::IRQ_CONTROLLER_DISTR_SIZE },
	          Memory_region { ::Board::Cpu_mmio::IRQ_CONTROLLER_CPU_BASE,
	                          ::Board::Cpu_mmio::IRQ_CONTROLLER_CPU_SIZE })
{
	::Board::Pic pic {};
}


void Board::Cpu::wake_up_all_cpus(void * ip)
{
	enum Function_id { CPU_ON = 0xC4000003 };

	unsigned long result = 0;
	for (unsigned i = 1; i < NR_OF_CPUS; i++) {
		asm volatile("mov x0, %1  \n"
		             "mov x1, %2  \n"
		             "mov x2, %3  \n"
		             "mov x3, %2  \n"
		             "smc #0      \n"
		             "mov %0, x0  \n"
		             : "=r" (result) : "r" (CPU_ON), "r" (i), "r" (ip)
		                      : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
		                        "x8", "x9", "x10", "x11", "x12", "x13", "x14");
	}
}


Hw::size_t Board::detect_ram_size()
{
	struct Dram : Genode::Mmio
	{
		struct Rank : Register<0x0, 32>
		{
			struct Dual_bank : Bitfield<0, 1> { };
			struct Banks     : Bitfield<2, 1> { };
			struct Row_bits  : Bitfield<4, 4> { };
			struct Page_size : Bitfield<8, 4> { };
		};

		bool     dual_bank() { return read<Rank::Dual_bank>() == 1; }
		unsigned bank_bits() { return read<Rank::Banks>() ? 3 : 2;  }
		unsigned row_bits()  { return read<Rank::Row_bits>() + 1;   }

		unsigned page_size()
		{
			/*
			 * Page_size is calculated by fls -> hence - 1, +4 is taken from U-boot
			 * comes down to +3
			 */
			return 1u << (read<Rank::Page_size>() + 3);
		}

		Hw::size_t size()
		{
			return (1ul << (row_bits() + bank_bits())) * page_size();
		}

		Dram(Hw::addr_t const base) : Mmio(base) { }
	};

	Dram  rank0 { ::Board::DRAMCOM_BASE };
	Dram  rank1 { ::Board::DRAMCOM_BASE + 0x4 };

	Hw::size_t size = rank0.size();

	if (rank0.dual_bank()) size += rank1.size();

	return size;
}
