/*
 * \brief  Allwinner A64 PIO driver
 * \author Norman Feske
 * \date   2021-04-14
 */

#ifndef _TYPES_H_
#define _TYPES_H_

/* local includes */
#include <common_types.h>

namespace Pio_driver {

	using namespace Pin_driver;

	struct Bank;
	struct Pin_id;
}


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

#endif /* _TYPES_H_ */
