/*
 * \brief  Test for accessing GPIO pin on Allwinner A64 SoC
 * \author Norman Feske
 * \date   2021-03-12
 *
 * An interrupt is generated each time when connecting the GPIO pin PB2 (Euler
 * connector pin 27) via a resistor of a few hundred Ohm to 5V (pin 8).
 */

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <base/attached_io_mem_dataspace.h>
#include <util/mmio.h>
#include <irq_session/connection.h>

namespace Test {

	using namespace Genode;

	struct Main;
}


struct Test::Main
{
	Env &_env;

	static constexpr size_t PIO_DS_SIZE = 0x400u;

	Attached_io_mem_dataspace _pio_ds { _env, 0x1c20800u, PIO_DS_SIZE };

	struct Pio : Mmio<0x218>
	{
		struct Pb_cfg0 : Register<0x24, 32>
		{
			struct Pb2_select : Bitfield<8, 3>
			{
				enum { EINT2 = 6 };
			};
		};

		struct Pb_eint_status : Register<0x214, 32>
		{
			struct Pb2 : Bitfield<2, 1> { };
		};

		struct Pb_eint_ctl : Register<0x210, 32>
		{
			struct Pb2 : Bitfield<2, 1> { };
		};

		struct Pb_pull0 : Register<0x40, 32>
		{
			enum { PULL_DOWN = 2 };

			struct Pb2 : Bitfield<4, 2> { };
		};

		Pio(Byte_range_ptr const &range) : Mmio(range)
		{
			/* enable pull down to avoid high-impedance (undefined) signal */
			write<Pb_pull0::Pb2>(Pb_pull0::PULL_DOWN);

			/* configure PB2 pin to interrupt mode */
			write<Pb_cfg0::Pb2_select>(Pb_cfg0::Pb2_select::EINT2);

			/* enable interrupt delivery from device to GIC */
			write<Pb_eint_ctl::Pb2>(1);
		}

		void clear_pb2_status()
		{
			write<Pb_eint_status::Pb2>(1);
		}
	};

	Pio _pio { {_pio_ds.local_addr<char>(), PIO_DS_SIZE} };

	enum { PB_EINT = 43 };

	Irq_connection _irq { _env, unsigned(PB_EINT) };

	unsigned _count = 0;

	void _handle_irq()
	{
		log("interrupt ", _count++, " occurred");

		_pio.clear_pb2_status();

		_irq.ack_irq();
	}

	Signal_handler<Main> _irq_handler { _env.ep(), *this, &Main::_handle_irq };

	Main(Env &env) : _env(env)
	{
		_irq.sigh(_irq_handler);
		_handle_irq();
	}
};


void Component::construct(Genode::Env &env)
{
	static Test::Main main(env);
}

