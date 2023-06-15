/*
 * \brief  Driver for the PinePhone's volume buttons
 * \author Norman Feske
 * \date   2023-06-15
 */

/* Genode includes */
#include <base/component.h>
#include <input/keycodes.h>
#include <event_session/connection.h>
#include <platform_session/device.h>

namespace Button_driver {

	using namespace Genode;

	struct Keyadc;
	struct Main;
}


struct Button_driver::Keyadc
{
	struct Mmio : Platform::Device::Mmio
	{
		struct Ctrl : Register<0x0, 32>
		{
			struct En : Bitfield<0, 1> { };
		};

		struct Intc : Register<0x4, 32> /* interrupt control */
		{
			struct Down_en : Bitfield<1, 1> { };
			struct Up_en   : Bitfield<4, 1> { };
		};

		struct Ints : Register<0x8, 32> /* interrupt status */ { };

		struct Data : Register<0xc, 32>
		{
			struct Bits : Bitfield<0, 5> { };
		};

		using Platform::Device::Mmio::Mmio;

	} _mmio;

	Platform::Device::Irq _irq;

	Keyadc(Platform::Device &device, Signal_context_capability irq_sigh)
	:
		_mmio(device), _irq(device)
	{
		_irq.sigh(irq_sigh);

		/* enable device */
		_mmio.write<Mmio::Ctrl::En>(1);

		/* enable key-up and key-down interrupts */
		_mmio.write<Mmio::Intc::Down_en>(1);
		_mmio.write<Mmio::Intc::Up_en>(1);

		/* clear all interrupt-status bits */
		_mmio.write<Mmio::Ints>(~0);
	}

	template <typename FN>
	void handle_data(FN const &fn)
	{
		fn(uint8_t(_mmio.read<Mmio::Data::Bits>()));

		/* clear interrupt-status bits */
		_mmio.write<Mmio::Ints>(_mmio.read<Mmio::Ints>());

		_irq.ack();
	}
};


struct Button_driver::Main
{
	Env &_env;

	Event::Connection _event { _env };

	struct Key_state { bool volume_down, volume_up; } _key_state { };

	Platform::Connection _platform { _env };

	Platform::Device _keyadc_device { _platform };

	Signal_handler<Main> _keyadc_handler {
		_env.ep(), *this, &Main::_handle_keyadc };

	void _handle_keyadc()
	{
		_keyadc.handle_data([&] (uint8_t data)
		{
			Key_state const orig = _key_state;

			_key_state = { };

			if      (data < 0x08) _key_state.volume_up   = true;
			else if (data < 0x10) _key_state.volume_down = true;

			_event.with_batch([&] (Event::Session_client::Batch &batch) {

				using namespace Input;

				auto submit_change = [&] (bool orig, bool curr, Input::Keycode key)
				{
					if (!orig &&  curr) batch.submit( Press   { key });
					if ( orig && !curr) batch.submit( Release { key });
				};

				submit_change(orig.volume_up,   _key_state.volume_up,   KEY_VOLUMEUP);
				submit_change(orig.volume_down, _key_state.volume_down, KEY_VOLUMEDOWN);
			});
		});
	}

	Keyadc _keyadc { _keyadc_device, _keyadc_handler };

	Main(Env &env) : _env(env) { }
};


void Component::construct(Genode::Env &env)
{
	static Button_driver::Main main(env);
}

