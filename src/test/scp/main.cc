/*
 * \brief  Test for accessing the system-control processor (SCP)
 * \author Norman Feske
 * \date   2022-05-11
 */

/* Genode includes */
#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <scp_session/connection.h>

namespace Test {

	using namespace Genode;

	struct Main;
}


struct Test::Main
{
	Env &_env;

	Attached_rom_dataspace _config { _env, "config" };

	Scp::Connection _scp { _env };

	Main(Env &env) : _env(env)
	{
		auto with_raw_attr_value = [&] (auto const &attr_name, auto const &fn)
		{
			_config.xml().for_each_attribute([&] (Xml_attribute const &attr) {
				if (attr.name() == attr_name)
					attr.with_raw_value(fn); });
		};

		with_raw_attr_value("program",
			[&] (char const *program, size_t program_len) {
				_scp.execute(
					[&] (char *buf, size_t buf_len) {
						if (program_len > buf_len) {
							error("SCP program exceeds maximum request size");
							return 0UL;
						} else {
							copy_cstring(buf, program, buf_len);
							return program_len;
						}
					},

					[&] (char const *result, size_t len) {
						log("SCP response: ", Cstring(result, len));
					},

					[&] (Scp::Execute_error e) {
						switch (e) {
						case Scp::Execute_error::REQUEST_TOO_LARGE:
							error("unable to execute too large SCP request");
							break;
						case Scp::Execute_error::RESPONSE_TOO_LARGE:
							error("unable to retrieve too large SCP response");
							break;
						}
					}
				);
			}
		);
	}
};


void Component::construct(Genode::Env &env) { static Test::Main main(env); }
