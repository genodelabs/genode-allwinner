/*
 * \brief  Allwinner A64 PIO driver
 * \author Norman Feske
 * \date   2021-04-14
 */

/* Genode includes */
#include <util/list_model.h>
#include <base/component.h>
#include <base/heap.h>
#include <base/attached_rom_dataspace.h>
#include <os/session_policy.h>
#include <pin_state_session/component.h>
#include <pin_control_session/component.h>

/* local includes */
#include <pio.h>

namespace Pio_driver {

	struct Pin_declaration;
	struct Irq_handler;
	struct Main;
}


/**
 * Pin information obtained from the configuration
 */
struct Pio_driver::Pin_declaration : List_model<Pin_declaration>::Element
{
	Name const name;

	/**
	 * Expected by 'update_list_model_from_xml'
	 */
	static bool type_matches(Xml_node const &node)
	{
		bool const valid_type = node.has_type("in")
		                     || node.has_type("out")
		                     || node.has_type("select");

		return valid_type && node.has_attribute("name");
	}

	/**
	 * Expected by 'update_list_model_from_xml'
	 */
	bool matches(Xml_node const &node) const
	{
		return type_matches(node)
		    && (name == Name::from_xml(node))
		    && (attr.function.direction() == Function::from_xml(node).direction());
	}

	Constructible<Pin_id> id { };

	Attr attr = Attr::disabled();

	unsigned ref_count = 0;

	Pin_declaration(Xml_node const &node) : name(Name::from_xml(node)) { }

	void update_from_xml(Xml_node const &pin, Pio &pio)
	{
		try {
			Pin_id const new_id = Pin_id::from_xml(pin);

			attr = Attr::from_xml(pin);

			/* reset original pin if physical location has changed */
			bool const id_changed = (id.constructed() && *id != new_id);
			if (id_changed)
				pio.configure(*id, Attr::disabled());

			id.construct(new_id);

			/*
			 * Update device configuration, except for <out> pins without an
			 * 'default' attribute. The configuration of such pins as output
			 * depends on the lifetime of a corresponding 'Pin_control' session.
			 */

			bool const output = (attr.function.value == Function::OUTPUT);
			bool const on_demand_output = (output && attr.out_on_demand);

			if (!on_demand_output)
				pio.configure(new_id, attr);

			bool const configured_default_output = (output && !attr.out_on_demand);

			if (ref_count == 0 && configured_default_output)
				pio.state(new_id, attr.default_state);
		}
		catch (... /* malformed pin declaration */ ) {

			if (id.constructed())
				pio.configure(*id, Attr::disabled());

			id.destruct();
			attr = Attr::disabled();
		}
	}

	bool matches(Pin_id pin_id, Pin::Direction dir) const
	{
		return id.constructed() && (*id == pin_id) && (dir == attr.function.direction());
	}
};


struct Pio_driver::Irq_handler
{
	struct Fn : Interface { virtual void handle_irq() = 0; };

	using Device = Platform::Device;

	Device::Irq _irq;

	Fn &_fn; /* Main::handle_irq */

	Signal_handler<Irq_handler> _handler;

	void _handle_irq()
	{
		_fn.handle_irq();

		/* acknowledge at GIC */
		_irq.ack();
	}

	Irq_handler(Env &env, Device &device, Device::Irq::Index index, Fn &fn)
	:
		_irq(device, index), _fn(fn),
		_handler(env.ep(), *this, &Irq_handler::_handle_irq)
	{
		_irq.sigh(_handler);
	}
};


struct Pio_driver::Main : Pin::Driver<Pin_id>, Irq_handler::Fn
{
	Env &_env;

	Platform::Connection _platform { _env };
	Platform::Device     _device   { _platform };
	Pio                  _pio      { _device };

	using Irq = Platform::Device::Irq;

	Irq_handler _irq_port_b { _env, _device, Irq::Index { 0 }, *this };
	Irq_handler _irq_port_g { _env, _device, Irq::Index { 1 }, *this };
	Irq_handler _irq_port_h { _env, _device, Irq::Index { 2 }, *this };
	Irq_handler _irq_port_l { _env, _device, Irq::Index { 3 }, *this };

	/**
	 * Irq_handler::Fn interface
	 */
	void handle_irq() override { deliver_pin_irqs(); }

	Attached_rom_dataspace _config { _env, "config" };

	Signal_handler<Main> _config_handler {
		_env.ep(), *this, &Main::_handle_config };

	void _handle_config();

	Heap _heap { _env.ram(), _env.rm() };

	Pin_state::Root<Pin_id>   _pin_state_root   { _env, _heap, *this };
	Pin_control::Root<Pin_id> _pin_control_root { _env, _heap, *this };
	Pin::Irq_root<Pin_id>     _irq_root         { _env, _heap, *this };

	List_model<Pin_declaration> _pins { };

	template <typename FN>
	void _with_pin_declaration(Pin_id id, Pin::Direction dir, FN const &fn)
	{
		_pins.for_each([&] (Pin_declaration &pin) {
			if (pin.matches(id, dir))
				fn(pin); });
	}

	bool pin_state(Pin_id id) const override
	{
		return _pio.state(id);
	}

	void pin_state(Pin_id id, Pin::Level level) override
	{
		_pio.state(id, level);
	}

	Pin_id assigned_pin(Session_label, Pin::Direction) const override;

	void acquire_pin(Pin_id id, Pin::Direction dir) override
	{
		_with_pin_declaration(id, dir, [&] (Pin_declaration &pin) {

			pin.ref_count++;

			if (pin.attr.output()) {
				if (pin.attr.out_on_demand)
					_pio.configure(id, pin.attr);
				else
					_pio.state(id, pin.attr.default_state);
			}

			if (pin.attr.irq())
				_pio.configure(id, pin.attr);
		});
	}

	void release_pin(Pin_id id, Pin::Direction direction) override
	{
		_with_pin_declaration(id, direction, [&] (Pin_declaration &pin) {

			if (--pin.ref_count > 0)
				return;

			if (pin.attr.output()) {
				if (pin.attr.out_on_demand)
					_pio.configure(id, Attr::disabled());
				else
					_pio.state(id, pin.attr.default_state);
			}

			if (pin.attr.irq())
				_pio.configure(id, Attr::disabled());
		});
	}

	void irq_enabled(Pin_id id, bool enabled) override
	{
		_pio.irq_enabled(id, enabled);
	}

	bool irq_pending(Pin_id id) const override
	{
		return _pio.irq_pending(id);
	}

	void ack_irq(Pin_id id) override
	{
		_pio.clear_irq_status(id);
	}

	Main(Env &env) : _env(env)
	{
		/* subscribe to config updates and import initial config */
		_config.sigh(_config_handler);
		_handle_config();
	}
};


Pio_driver::Pin_id Pio_driver::Main::assigned_pin(Session_label label,
                                                  Pin::Direction dir) const
{
	/*
	 * \throw Service_denied
	 */
	Session_policy policy { label, _config.xml() };

	Name const name { policy.attribute_value("pin", Name::String()) };

	Constructible<Pin_id> pin_id { };

	_pins.for_each([&] (Pin_declaration const &pin) {
		if (pin.name == name && pin.attr.function.direction() == dir)
			pin_id.construct(*pin.id); });

	if (!pin_id.constructed()) {
		char const *node_type = (dir == Pin::Direction::IN) ? "<in>" : "<out>";
		warning("missing ", node_type, " pin assignment for session '", label, "'");
		throw Service_denied();
	}

	return *pin_id;
}


void Pio_driver::Main::_handle_config()
{
	_config.update();

	/*
	 * Update pin declarations
	 */

	auto create = [&] (Xml_node const &node) -> Pin_declaration &
	{
		return *new (_heap) Pin_declaration(node);
	};

	auto destroy = [&] (Pin_declaration &pin)
	{
		if (pin.id.constructed())
			_pio.configure(*pin.id, Attr::disabled());

		Genode::destroy(_heap, &pin);
	};

	auto update = [&] (Pin_declaration &pin, Xml_node node)
	{
		pin.update_from_xml(node, _pio);
	};

	update_list_model_from_xml(_pins, _config.xml(), create, destroy, update);

	/*
	 * Re-assign sessions to pins
	 */

	_pin_state_root  .update_assignments();
	_pin_control_root.update_assignments();
	_irq_root        .update_assignments();
}


void Component::construct(Genode::Env &env)
{
	static Pio_driver::Main main(env);
}
