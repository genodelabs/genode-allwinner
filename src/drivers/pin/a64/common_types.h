/*
 * \brief  Common pin driver types
 * \author Norman Feske
 * \date   2021-04-14
 */

#ifndef _COMMON_TYPES_H_
#define _COMMON_TYPES_H_

/* Genode includes */
#include <util/xml_node.h>
#include <base/log.h>

namespace Pin_driver {

	using namespace Genode;

	struct Name;
	struct Index;
	struct Function;
	struct Pull;
	struct Irq_trigger;
	struct Attr;
	struct Irq_handler;
}


struct Pin_driver::Name
{
	using String = Genode::String<32>;

	String string;

	static Name from_xml(Xml_node const &node)
	{
		return { node.attribute_value("name", String()) };
	}

	bool operator == (Name const &other) const { return other.string == string; }
};


/**
 * Pin index within bank
 */
struct Pin_driver::Index
{
	unsigned value;

	class Invalid : Exception { };

	static Index from_xml(Xml_node node)
	{
		if (!node.has_attribute("index")) {
			warning("pin declarion lacks 'index' attribute: ", node);
			throw Invalid();
		}
		return Index { node.attribute_value("index", 0u) };
	}
};


struct Pin_driver::Function
{
	enum Value { INPUT = 0, OUTPUT = 1, FN2 = 2, FN3     = 3,
	             FN4   = 4, FN5    = 5, IRQ = 6, DISABLE = 7 } value;

	Function() = delete;

	class Invalid : Exception { };

	static Function from_xml(Xml_node node)
	{
		if (node.has_type("in"))
			return node.has_attribute("irq") ? Function { IRQ }
			                                 : Function { INPUT };

		if (node.has_type("out"))
			return Function { OUTPUT };

		if (!node.has_type("select"))
			throw Invalid();

		unsigned const n = node.attribute_value("function", (unsigned)DISABLE);

		if (n > DISABLE) {
			warning("function number out of range: ", node);
			throw Invalid();
		}

		if (n == INPUT || n == IRQ) {
			warning("function number ", n, " reserved for <in> pins");
			throw Invalid();
		}

		if (n == OUTPUT) {
			warning("function number ", n, " reserved for <out> pins");
			throw Invalid();
		}

		return Function { (Value)n };
	};

	Pin::Direction direction() const
	{
		return (value == OUTPUT) ? Pin::Direction::OUT : Pin::Direction::IN;
	}

	void print(Output &out) const
	{
		switch (value)
		{
			case INPUT:
				Genode::print(out, "INPUT");
				break;
			case OUTPUT:
				Genode::print(out, "OUTPUT");
				break;
			case IRQ:
				Genode::print(out, "IRQ");
				break;
			case DISABLE:
				Genode::print(out, "DISABLE");
				break;
			default:
				Genode::print(out, (unsigned)value);
				break;
		}
	}
};


struct Pin_driver::Pull
{
	enum Value { DISABLE = 0, UP = 1, DOWN = 2 } value;

	class Invalid : Exception { };

	static Pull from_xml(Xml_node node)
	{
		if (!node.has_attribute("pull"))
			return Pull { DISABLE };

		auto const value = node.attribute_value("pull", String<10>());
		if (value == "up")
			return Pull { UP };

		if (value == "down")
			return Pull { DOWN };

		warning("invalid pull attribute value: ", node);
		throw Invalid();
	};
};


struct Pin_driver::Irq_trigger
{
	enum Value { RISING = 0, FALLING = 1, HIGH = 2, LOW = 3, EDGES = 4 } value;

	class Invalid : Exception { };

	static Irq_trigger from_xml(Xml_node node)
	{
		if (!node.has_attribute("irq"))
			return Irq_trigger { RISING };

		auto const value = node.attribute_value("irq", String<10>());

		if (value == "rising")  return Irq_trigger { RISING };
		if (value == "falling") return Irq_trigger { FALLING };
		if (value == "high")    return Irq_trigger { HIGH };
		if (value == "low")     return Irq_trigger { LOW };
		if (value == "edges")   return Irq_trigger { EDGES };

		warning("invalid irq attribute value: ", node);
		throw Invalid();
	};
};


/**
 * Pin attributes
 */
struct Pin_driver::Attr
{
	Pull        pull;
	Function    function;
	Irq_trigger irq_trigger;
	bool        out_on_demand;  /* activate output on access by 'Pin_control' client */
	Pin::Level  default_state;

	Attr() = delete;

	bool output() const { return function.value == Function::OUTPUT; }
	bool irq()    const { return function.value == Function::IRQ; }

	static Attr from_xml(Xml_node const &node)
	{
		auto default_state_from_xml = [] (Xml_node const &node)
		{
			if (!node.has_attribute("default"))
				return Pin::Level::HIGH_IMPEDANCE;

			return node.attribute_value("default", false)
			       ? Pin::Level::HIGH : Pin::Level::LOW;
		};

		return { .pull          = Pull::from_xml(node),
		         .function      = Function::from_xml(node),
		         .irq_trigger   = Irq_trigger::from_xml(node),
		         .out_on_demand = !node.has_attribute("default"),
		         .default_state = default_state_from_xml(node) };
	}

	static Attr disabled()
	{
		return { .pull          = { Pull::DISABLE },
		         .function      = { Function::DISABLE },
		         .irq_trigger   = { Irq_trigger::RISING },
		         .out_on_demand = false,
		         .default_state = Pin::Level::HIGH_IMPEDANCE };
	}
};


/**
 * Utility for irq handling
 */
struct Pin_driver::Irq_handler
{
	struct Fn : Interface { virtual void handle_irq() = 0; };

	using Device = Platform::Device;

	Device::Irq _irq;

	Fn &_fn;

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

#endif /* _COMMON_TYPES_H_ */
