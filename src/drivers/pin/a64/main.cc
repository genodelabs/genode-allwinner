/*
 * \brief  Allwinner A64 PIO driver
 * \author Norman Feske
 * \date   2021-04-14
 */

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <platform_session/device.h>

namespace Pio_drv {

	using namespace Genode;

	struct Main;
}


struct Pio_drv::Main
{
	Env &_env;

	Platform::Connection _platform { _env };

	Platform::Device _device { _platform };

	struct Pio : Platform::Device::Mmio
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

		Pio(Platform::Device &device) : Mmio(device)
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

	Pio _pio { _device };

	Main(Env &env) : _env(env)
	{
		/* print state of PB2 signal in busy loop */
		while (true)
			log("PB2: ", _pio.pb2_state());
	}
};


void Component::construct(Genode::Env &env)
{
	static Pio_drv::Main main(env);
}

