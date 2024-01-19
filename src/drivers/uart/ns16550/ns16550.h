/*
 * \brief  Driver for the NS16550 UART
 * \author Norman Feske
 * \author Sebastian Sumpf
 * \date   2021-01-21
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _NS16550_H_
#define _NS16550_H_

/* Genode includes */
#include <platform_session/device.h>

namespace Genode { class Ns16550_uart; }


class Genode::Ns16550_uart : Platform::Device::Mmio<0x18>
{
	private:

		/**
		 * Divisor latch low register
		 */
		struct Dll : Register<0x00, 32>
		{
			struct Data : Bitfield<0, 8> { };
		};

		/**
		 * Divisor latch high register
		 */
		struct Dlh : Register<0x04, 32>
		{
			struct Data : Bitfield<0, 8> { };
		};

		/**
		 * Interrupt enable register
		 */
		struct Ier : Register<0x4, 32>
		{
			/* enable received data available interrupt */
			struct Erbfi : Bitfield<0, 1> { };
		};

		/**
		 * Transmitter holding register
		 */
		struct Thr : Register<0x00, 32>
		{
			struct Data : Bitfield<0,8> { };
		};

		struct Iir : Register<0x8, 32> { };
		/**
		 * Fifo control register
		 */
		struct Fcr : Register<0x08, 32>
		{
			struct Enable : Bitfield<0, 1> { };
			struct Reset_rx : Bitfield<1, 1> { };
			struct Reset_tx : Bitfield<2, 1> { };
		};

		/**
		 * Line control register
		 */
		struct Lcr : Register<0x0c, 32>
		{
			/* data length select */
			struct Dls : Bitfield<0, 2> { enum { EIGHT_BIT = 0x3 }; };
			/* divisor latch bit */
			struct Dlab : Bitfield<7, 1> { };
		};

		/**
		 * Modem control register
		 */
		struct Mcr : Register<0x10, 32>
		{
			struct Dts : Bitfield<0, 1> { };
			struct Rts : Bitfield<1, 1> { };
		};

		/**
		 * Line-status register
		 */
		struct Lsr : Register<0x14, 32>
		{
			struct Thr_empty : Bitfield<5,1> { };
			struct Dr        : Bitfield<0, 1> { };
		};

	public:

		Ns16550_uart(Platform::Device &device, uint32_t clock, uint32_t baud)
		: Mmio(device)
		{
			/* wait for pending transmitt */
			while (read<Lsr::Thr_empty>() == 0);

			/* disable IRQs */
			write<Ier>(0);

			write<Mcr::Dts>(0);
			write<Mcr::Rts>(0);

			/* enable and reset FIFOs */
			Fcr::access_t fifo = 0;
			Fcr::Enable::set(fifo, 1);
			Fcr::Reset_rx::set(fifo, 1);
			Fcr::Reset_tx::set(fifo, 1);
			write<Fcr>(fifo);

			/* 8N1 */
			write<Lcr::Dls>(Lcr::Dls::EIGHT_BIT);

			/* select divisor bank and set baud rate */
			write<Lcr::Dlab>(1);
			uint32_t div = clock / (16 * baud);
			write<Dll::Data>(div);
			write<Dlh::Data>(div >> 8);
			write<Lcr::Dlab>(0);
		}

		void enable_irq()
		{
			write<Ier::Erbfi>(1);
		}

		void put_char(char const c)
		{
			while (read<Lsr::Thr_empty>() == 0);

			write<Thr::Data>(c);
		}

		bool char_avail()
		{
			return read<Lsr::Dr>() == 1;
		}

		char get_char()
		{
			return (char)read<Thr::Data>();
		}
};

#endif /* _NS16550_H_ */
