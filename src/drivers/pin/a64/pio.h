/*
 * \brief  Device interface of the Allwinner A64 PIO driver
 * \author Norman Feske
 * \date   2021-04-14
 */

#ifndef _PIO_H_
#define _PIO_H_

/* Genode includes */
#include <platform_session/device.h>

/* local includes */
#include <types.h>

namespace Pio_driver { struct Pio; }


struct Pio_driver::Pio
{
	Platform::Device::Mmio _mmio;

	struct Io_bank : Genode::Mmio
	{
		using Genode::Mmio::Mmio;

		struct Cfg  : Register_array<0x0,  32, 32, 4> { };
		struct Data : Register_array<0x10, 32, 32, 1> { };
		struct Pull : Register_array<0x1c, 32, 32, 2> { };

		void configure(Index index, Attr const &attr)
		{
			write<Cfg> (attr.function.value, index.value);
			write<Pull>(attr.pull.value,     index.value);
		}

		bool state(Index index) const
		{
			return read<Data>(index.value);
		}

		void state(Index index, bool enabled)
		{
			write<Data>(enabled, index.value);
		}
	};

	struct Irq_regs : Genode::Mmio
	{
		enum Id { B, G, H, NUM, UNDEFINED };

		using Genode::Mmio::Mmio;

		struct Cfg     : Register_array<0x0,  32, 32, 4> { };
		struct Control : Register_array<0x10, 32, 32, 1> { };
		struct Status  : Register_array<0x14, 32, 32, 1> { };

		void configure(Index index, Attr const &attr)
		{
			write<Cfg>(attr.irq_trigger.value, index.value);
		}

		void clear_irq_status(Index index)
		{
			write<Status>(1, index.value);
		}

		bool irq_pending(Index index) const
		{
			return read<Status>(index.value);
		}

		/**
		 * Enable/disable IRQ delivery to GIC
		 */
		void irq_enabled(Index index, bool enabled)
		{
			write<Control>(enabled, index.value);
		}
	};

	addr_t const _base;

	Constructible<Io_bank> _io_banks [Bank::NUM];

	Irq_regs _irq_regs_b { _base + 0x200 };
	Irq_regs _irq_regs_g { _base + 0x220 };
	Irq_regs _irq_regs_h { _base + 0x240 };

	template <typename PIO, typename FN>
	static void _with_irq_regs(PIO &pio, Pin_id id, FN const &fn)
	{
		switch (id.bank.value) {
		case Bank::B: fn(pio._irq_regs_b); break;
		case Bank::G: fn(pio._irq_regs_g); break;
		case Bank::H: fn(pio._irq_regs_h); break;
		default: break;
		};
	}

	Pio(Platform::Device &device)
	:
		_mmio(device), _base((addr_t)_mmio.local_addr<void>())
	{
		for (unsigned i = Bank::B; i < Bank::NUM; i++)
			_io_banks [i].construct(_base + 0x24*i);
	}

	void configure(Pin_id id, Attr const &attr)
	{
		_io_banks [id.bank.value]->configure(id.index, attr);

		_with_irq_regs(*this, id, [&] (Irq_regs &irq_regs) {
			irq_regs.configure(id.index, attr); });
	}

	bool state(Pin_id id) const
	{
		return _io_banks[id.bank.value]->state(id.index);
	}

	void state(Pin_id id, bool enabled)
	{
		_io_banks[id.bank.value]->state(id.index, enabled);
	}

	void clear_irq_status(Pin_id id)
	{
		_with_irq_regs(*this, id, [&] (Irq_regs &irq_regs) {
			irq_regs.clear_irq_status(id.index); });
	}

	void irq_enabled(Pin_id id, bool enabled)
	{
		_with_irq_regs(*this, id, [&] (Irq_regs &irq_regs) {
			irq_regs.irq_enabled(id.index, enabled); });
	}

	bool irq_pending(Pin_id id) const
	{
		bool pending = false;

		_with_irq_regs(*this, id, [&] (Irq_regs const &regs) {
			pending = regs.irq_pending(id.index); });

		return pending;
	}
};

#endif /* _PIO_H_ */
