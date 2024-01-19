/*
 * \brief  Test for accessing GPIO pin on Allwinner A64 SoC
 * \author Norman Feske
 * \date   2021-03-12
 *
 * Drive the state of an LED connected over a resistor of a few hundred
 * Ohm between the Euler-connector pin 27 (LED anode) and pin 37 (LED cathode).
 * The test program toggles the pin 27 between GND and 5V, letting the LED
 * blink.
 */

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <base/attached_io_mem_dataspace.h>
#include <util/mmio.h>
#include <timer_session/connection.h>

namespace Test {

	using namespace Genode;

	struct Main;
}


struct Test::Main
{
	Env &_env;

	static constexpr size_t PIO_DS_SIZE = 0x400u;

	Attached_io_mem_dataspace _pio_ds { _env, 0x1c20800u, PIO_DS_SIZE };

	struct Pio : Mmio<0x38>
	{
		struct Pb_cfg0 : Register<0x24, 32>
		{
			struct Pb2_select : Bitfield<8, 3>
			{
				enum { OUT = 1 };
			};
		};

		struct Pb_data : Register<0x34, 32>
		{
			struct Pb2 : Bitfield<2, 1> { };
		};

		Pio(Byte_range_ptr const &range) : Mmio(range)
		{
			/* configure PB2 pin to output mode */
			write<Pb_cfg0::Pb2_select>(Pb_cfg0::Pb2_select::OUT);
		}

		void toggle_pb2()
		{
			bool const value = read<Pb_data::Pb2>();

			/* write back inverted value */
			write<Pb_data::Pb2>(!value);
		}
	};

	Pio _pio { {_pio_ds.local_addr<char>(), PIO_DS_SIZE} };

	Timer::Connection _timer { _env };

	void _handle_timeout(Duration)
	{
		_pio.toggle_pb2();
	}

	Timer::Periodic_timeout<Main> _timout_handler {
		_timer, *this, &Main::_handle_timeout, Microseconds { 250*1000 } };

	Main(Env &env) : _env(env) { }
};


void Component::construct(Genode::Env &env)
{
	static Test::Main main(env);
}

