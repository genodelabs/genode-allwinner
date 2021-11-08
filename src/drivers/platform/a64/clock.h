/*
 * \brief  Clock interface for platform driver
 * \author Stefan Kalkowski
 * \author Norman Feske
 * \date   2020-06-12
 */

/*
 * Copyright (C) 2020-2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <types.h>
#include <named_registry.h>

namespace Driver {

	using namespace Genode;

	class Clock;
	class Fixed_clock;
	class Fixed_divider;

	using Clocks = Named_registry<Clock>;
}


class Driver::Clock : Clocks::Element, Interface
{
	private:

		/* friendships needed to make 'Clocks::Element' private */
		friend class Clocks::Element;
		friend class Avl_node<Clock>;
		friend class Avl_tree<Clock>;

		Switch<Clock> _switch { *this, &Clock::_enable, &Clock::_disable };

	protected:

		virtual void _enable()  { }
		virtual void _disable() { }

	public:

		using Name = Clocks::Element::Name;
		using Clocks::Element::name;
		using Clocks::Element::Element;

		virtual void          set_rate(unsigned long rate) = 0;
		virtual unsigned long get_rate()             const = 0;
		virtual void          set_parent(Name) { }

		void enable()  { _switch.use();   }
		void disable() { _switch.unuse(); }
};


class Driver::Fixed_clock : public Driver::Clock
{
	private:

		unsigned long _rate;

	public:

		Fixed_clock(Clocks &clocks, Name const &name, unsigned long rate)
		:
			Clock(clocks, name), _rate(rate)
		{ }

		void          set_rate(unsigned long) override { }
		unsigned long get_rate()        const override { return _rate; }
};


class Driver::Fixed_divider : public Driver::Clock
{
	private:

		Clock &_parent;

		unsigned const _divider;

	public:

		Fixed_divider(Clocks     &clocks,
		              Name const &name,
		              Clock      &parent,
		              unsigned    divider)
		:
			Clock(clocks, name), _parent(parent), _divider(divider)
		{ }

		void set_rate(unsigned long) override { }

		unsigned long get_rate() const override
		{
			return _parent.get_rate() / _divider;
		}
};

#endif /* _CLOCK_H_ */
