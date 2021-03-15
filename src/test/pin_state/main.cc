/*
 * \brief  Test for accessing GPIO pin on Allwinner A64 SoC
 * \author Norman Feske
 * \date   2021-03-12
 *
 * Shows the state change of the GPIO pin PB2 (Euler connector pin 27) when
 * connected via a resistor of a few hundred Ohm to 5V (pin 8).
 */

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <base/attached_io_mem_dataspace.h>
#include <util/mmio.h>

namespace Test {

	using namespace Genode;

	struct Main;
}


struct Test::Main
{
	Env &_env;

	Attached_io_mem_dataspace _pio_ds { _env, 0x1c20800u, 0x400u };

	struct Pio : Mmio
	{
		struct Pb_cfg0 : Register<0x24, 32>
		{
			struct Pb2_select : Bitfield<8, 3>
			{
				enum { IN = 0 };
			};
		};

		struct Pb_data : Register<0x34, 32>
		{
			struct Pb2 : Bitfield<2, 1> { };
		};

		struct Pb_pull0 : Register<0x40, 32>
		{
			enum { PULL_DOWN = 2 };

			struct Pb2 : Bitfield<4, 2> { };
		};

		Pio(addr_t base) : Mmio(base)
		{
			/* configure PB2 pin to input mode */
			write<Pb_cfg0::Pb2_select>(Pb_cfg0::Pb2_select::IN);

			/* enable pull down to avoid high-impedance (undefined) signal */
			write<Pb_pull0::Pb2>(Pb_pull0::PULL_DOWN);
		}

		bool pb2_state()
		{
			return read<Pb_data::Pb2>();
		}
	};

	Pio _pio { (addr_t)_pio_ds.local_addr<void>() };

	Main(Env &env) : _env(env)
	{
		/* print state of PB2 signal in busy loop */
		while (true)
			log("PB2: ", _pio.pb2_state());
	}
};


void Component::construct(Genode::Env &env)
{
	static Test::Main main(env);
}

