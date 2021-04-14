/*
 * \brief  Allwinner A64 PIO driver
 * \author Norman Feske
 * \date   2021-04-14
 */

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <platform_session/connection.h>

namespace Pio_drv {

	using namespace Genode;

	struct Main;
}


struct Pio_drv::Main
{
	Env &_env;

	Platform::Connection _platform { _env };

	Main(Env &env) : _env(env)
	{
	}
};


void Component::construct(Genode::Env &env)
{
	static Pio_drv::Main main(env);
}

