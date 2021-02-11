/*
 * \brief  Driver for the NS16550 UART
 * \author Norman Feske
 * \date   2021-01-21
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__DRIVERS__UART__NS16550_H_
#define _INCLUDE__DRIVERS__UART__NS16550_H_

/* Genode includes */
#include <util/mmio.h>

namespace Genode { class Ns16550_uart; }


class Genode::Ns16550_uart : Mmio
{
	private:

		/**
		 * Transmitter holding register
		 */
		struct Thr : Register<0x00, 32>
		{
			struct Data : Bitfield<0,8> { };
		};

		/**
		 * Line-status register
		 */
		struct Lsr : Register<0x14, 32>
		{
			struct Thr_empty : Bitfield<5,1> { };
		};

	public:

		Ns16550_uart(addr_t const base, uint32_t, uint32_t) : Mmio(base) { }

		void put_char(char const c)
		{
			while (read<Lsr::Thr_empty>() == 0);

			write<Thr::Data>(c);
		}
};

#endif /* _INCLUDE__DRIVERS__UART__NS16550_H_ */
