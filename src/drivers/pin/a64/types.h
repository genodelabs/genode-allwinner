/*
 * \brief  Allwinner A64 PIO driver
 * \author Norman Feske
 * \date   2021-04-14
 */

#ifndef _TYPES_H_
#define _TYPES_H_

/* Genode includes */
#include <util/xml_node.h>
#include <base/log.h>

namespace Pio_driver {

	using namespace Genode;

	struct Name;
	struct Bank;
	struct Index;
	struct Pin_id;
	struct Function;
	struct Pull;
	struct Irq_trigger;
	struct Attr;
}


struct Pio_driver::Name
{
	using String = Genode::String<32>;

	String string;

	static Name from_xml(Xml_node const &node)
	{
		return { node.attribute_value("name", String()) };
	}

	bool operator == (Name const &other) const { return other.string == string; }
};


struct Pio_driver::Bank
{
	enum Value { B = 1, C, D, E, F, G, H, L, NUM } value;

	class Invalid : Exception { };

	static Bank from_xml(Xml_node node)
	{
		typedef String<2> Name;
		Name name = node.attribute_value("bank", Name());

		if (name == "B") return { B };
		if (name == "C") return { C };
		if (name == "D") return { D };
		if (name == "E") return { E };
		if (name == "F") return { F };
		if (name == "G") return { G };
		if (name == "H") return { H };
		if (name == "L") return { L };

		warning("unknown PIO bank name '", name, "'");
		throw Invalid();
	};
};


/**
 * Pin index within bank
 */
struct Pio_driver::Index
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


/**
 * Unique physical identifier of a pin
 */
struct Pio_driver::Pin_id
{
	Bank  bank;
	Index index;

	Pin_id() = delete;

	static Pin_id from_xml(Xml_node const &node)
	{
		return { .bank  = Bank ::from_xml(node),
		         .index = Index::from_xml(node) };
	}

	bool operator == (Pin_id const &other) const
	{
		return other.bank.value  == bank.value
		    && other.index.value == index.value;
	}

	bool operator != (Pin_id const &other) const { return !(operator == (other)); }

	void print(Output &out) const
	{
		Genode::print(out, "P", Char('A' + (char)bank.value), index.value);
	}
};


struct Pio_driver::Function
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
};


struct Pio_driver::Pull
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


struct Pio_driver::Irq_trigger
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
struct Pio_driver::Attr
{
	Pull        pull;
	Function    function;
	Irq_trigger irq_trigger;
	bool        out_on_demand;  /* activate output on access by 'Pin_control' client */
	bool        default_state;

	Attr() = delete;

	bool output() const { return function.value == Function::OUTPUT; }
	bool irq()    const { return function.value == Function::IRQ; }

	static Attr from_xml(Xml_node const &node)
	{
		return { .pull          = Pull::from_xml(node),
		         .function      = Function::from_xml(node),
		         .irq_trigger   = Irq_trigger::from_xml(node),
		         .out_on_demand = !node.has_attribute("default"),
		         .default_state = node.attribute_value("default", false) };
	}

	static Attr disabled()
	{
		return { .pull          = { Pull::DISABLE },
		         .function      = { Function::DISABLE },
		         .irq_trigger   = { Irq_trigger::RISING },
		         .out_on_demand = false,
		         .default_state = false };
	}
};

#endif /* _TYPES_H_ */
