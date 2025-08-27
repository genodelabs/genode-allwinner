/*
 * \brief  Driver that connects a UART to a terminal session
 * \author Sebastian Sumpf
 * \date   2022-02-17
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/attached_io_mem_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <base/component.h>
#include <irq_session/connection.h>
#include <terminal_session/connection.h>

#include "ns16550.h"

namespace Uart {

	using namespace Genode;
	struct Main;

};


struct Uart::Main
{
	Env &_env;

	Attached_rom_dataspace _config { _env, "config" };
	bool _carriage_return {
		_config.node().attribute_value("carriage_return", false) };

	Platform::Connection  _platform { _env };
	Platform::Device      _device { _platform };
	Platform::Device::Irq _irq { _device };

	/* serial clock is 25MHz and 115200 baud  */
	Ns16550_uart         _uart { _device, 25000000, 115200 };
	Signal_handler<Main> _irq_handler { _env.ep(), *this, &Main::_handle_irq };

	Terminal::Connection _terminal { _env };
	Signal_handler<Main> _terminal_read { _env.ep(), *this, &Main::_handle_read };

	Main(Genode::Env &env) : _env(env)
	{
		_irq.sigh(_irq_handler);
		_terminal.read_avail_sigh(_terminal_read);
		_uart.enable_irq();
		_irq.ack();
	}

	void _handle_read()
	{
		while (_terminal.avail()) {

			char buf[32] { };
			size_t size = _terminal.read(buf, 32);
			for (size_t s = 0; s < size; s++) {
				_uart.put_char(buf[s]);
				if (_carriage_return && buf[s] == '\n')
					_uart.put_char('\r');
			}
		}
	}

	void _handle_irq()
	{
		_irq.ack();

		while (_uart.char_avail()) {
			char buf[100] { };
			unsigned count = 0;
			while (_uart.char_avail() && (count < sizeof(buf)))
				buf[count++] = _uart.get_char();

			_terminal.write(buf, count);
		}
	}
};


void Component::construct(Genode::Env &env)
{
	static Uart::Main uart(env);

}
