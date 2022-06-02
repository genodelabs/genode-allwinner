/*
 * \brief  Component for translating pin-state changes to input events
 * \author Norman Feske
 * \date   2021-04-23
 */

/* Genode includes */
#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <irq_session/connection.h>
#include <pin_state_session/connection.h>

namespace Pin_event {

	using namespace Genode;

	struct Main;
}


struct Pin_event::Main
{
	Env &_env;

	Pin_state::Connection _pin { _env };

	Irq_connection _irq { _env, 0 /* unused */ };

	Signal_handler<Main> _irq_handler {
		_env.ep(), *this, &Main::_handle_irq };

	void _handle_irq()
	{
		_irq.ack_irq();

		log("pin state: ", _pin.state());
	}

	/*
	 * Configuration
	 */

	Attached_rom_dataspace _config { _env, "config" };

	Signal_handler<Main> _config_handler {
		_env.ep(), *this, &Main::_handle_config };

	void _handle_config()
	{
		_config.update();
	}

	Main(Env &env) : _env(env)
	{
		_config.sigh(_config_handler);
		_handle_config();

		_irq.sigh(_irq_handler);
		_irq.ack_irq();
	}
};


void Component::construct(Genode::Env &env)
{
	static Pin_event::Main main(env);
}

