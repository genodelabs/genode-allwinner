/*
 * \brief  Audio interface driver for A64
 * \author Sebastian Sumpf
 * \date   2022-08-12
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <cpu/cache.h>
#include <base/component.h>
#include <base/heap.h>
#include <platform_session/device.h>
#include <platform_session/dma_buffer.h>
#include <util/touch.h>

#include <session.h>

using namespace Genode;

namespace Audio {
	struct Main;
	struct I2s_dma;
	class I2s;
	class Dma_engine;
}


class Audio::I2s : Platform::Device::Mmio
{
	private:

		struct Ap_control : Register<0x0, 32>
		{
			/* Global enable */
			struct Gen    : Bitfield<0, 1> { };
			struct Rxen   : Bitfield<1, 1> { };
			struct Txen   : Bitfield<2, 1> { };
			struct Sdo_en : Bitfield<8, 1> { };
		};

		struct Ap_format : Register<0x4, 32> { };

		struct Ap_int_status : Register<0xc, 32>
		{
			/* RX overrun */
			struct Rxo : Bitfield<1, 1> { };
			/* TX underrun */
			struct Txu : Bitfield<6, 1> { };
		};

		struct Ap_fifo : Register<0x14, 32>
		{
			struct Rxom : Bitfield<0, 2> { enum { SIGN_EXTENT = 1 }; };
			struct Txim : Bitfield<2, 1> { enum { LSB = 1 }; };

			/* flush FIFOs, must be called before enabling FIFOs */
			struct Frx : Bitfield<24, 1> { };
			struct Ftx : Bitfield<25, 1> { };
		};

		struct Ap_fifo_status : Register<0x18, 32>
		{
			struct Rxa : Bitfield<8 , 1> { };
			struct Txe : Bitfield<28, 1> { };
		};

		struct Ap_int : Register<0x1c, 32>
		{
			/* RX FIFO data available DRQ */
			struct Rx_drq  : Bitfield<3, 1> { };
			/* TX FIFO empty DRQ */
			struct Tx_drq  : Bitfield<7, 1> { };
		};

		struct Ap_clock : Register<0x24, 32>
		{
			struct Mclkdiv  : Bitfield<0, 4> { enum { DIV4 = 2 }; };
			struct Bclkdiv  : Bitfield<4, 3> { enum { DIV4 = 1 }; };
			struct Mclko_en : Bitfield<7, 1> { };
		};

		struct Ap_tx_counter : Register<0x28, 32> { };
		struct Ap_rx_counter : Register<0x2c, 32> { };

		/***********
		 ** DEBUG **
		 ***********/

		/*
		 * One sample - first access = left channel, second access = right channel
		 */
		struct Ap_rx_fifo : Register<0x10, 32> { };
		struct Ap_tx_fifo : Register<0x20, 32> { };

	public:

		I2s(Platform::Device &device)
		: Mmio(device)
		{
			/* disable in case I2S is still runnig */
			write<Ap_control::Gen>(0);
			write<Ap_control::Rxen>(0);
			write<Ap_control::Txen>(0);

			/* clocks */
			write<Ap_clock::Mclkdiv>(Ap_clock::Mclkdiv::DIV4);
			write<Ap_clock::Bclkdiv>(Ap_clock::Bclkdiv::DIV4);
			write<Ap_clock::Mclko_en>(1);

			/*
			 * Implies: I2S format (FMT), 16 bit word size BCLK (WSS), 16 bit sample
			 * resolution (SR)
			 */
			write<Ap_format>(0);

			/* setup and flush FIFOs */
			write<Ap_fifo::Rxom>(Ap_fifo::Rxom::SIGN_EXTENT);
			write<Ap_fifo::Txim>(Ap_fifo::Txim::LSB);
			write<Ap_fifo::Frx>(1);
			write<Ap_fifo::Ftx>(1);

			/* reset counter */
			write<Ap_rx_counter>(0);
			write<Ap_tx_counter>(0);

			/* enable FIFOs */
			write<Ap_control::Sdo_en>(1);
			write<Ap_control::Rxen>(1);
			write<Ap_control::Txen>(1);

			/* enable IRQs */
			write<Ap_int::Rx_drq>(1);
			write<Ap_int::Tx_drq>(1);
		}

		~I2s() { write<Ap_control::Gen>(0); }

		bool rx_overrun()
		{
			bool overrun = read<Ap_int_status::Rxo>();
			if (overrun) {
				write<Ap_int_status::Rxo>(1);
			}
			return overrun;
		}

		bool tx_underrun()
		{
			bool underrun = read<Ap_int_status::Txu>();
			if (underrun) {
				write<Ap_int_status::Txu>(1);
			}
			return underrun;
		}

		void enable() { write<Ap_control::Gen>(1); }
};


struct Audio::I2s_dma
{
	using Name = String<64>;

	class No_device_phys_addr : Genode::Exception { };

	addr_t addr { 0 };

	I2s_dma(Platform::Connection &platform)
	{
		platform.with_xml([&] (Xml_node & xml) {
			xml.for_each_sub_node("device", [&] (Xml_node node) {
				Name name = node.attribute_value("name", Name());
				if (name != "audio_interface") return;

				node.for_each_sub_node("io_mem", [&] (Xml_node node) {
					addr = node.attribute_value("phys_addr", 0UL);
				});
			});
		});

		if (addr == 0) {
			error("DMA addess of audio interface not found. Try adding\n"
			      "<policy info=\"yes\" ...>\n"
			      "to platform driver configuration.");
			throw No_device_phys_addr();
		}
	}

	addr_t rx_addr() const { return addr + 0x10; }
	addr_t tx_addr() const { return addr + 0x20; }
};


class Audio::Dma_engine : Platform::Device::Mmio
{
	private:

		struct Irq_enable  : Register<0x0, 32>  { };
		struct Irq_pending : Register<0x10, 32> { };

		struct Security : Register<0x20, 32>
		{
			struct Channels : Bitfield<0, 8> { };
		};

		struct Auto_gate : Register<0x28, 32>
		{
			struct Mclk_circuit : Bitfield<2, 1> { };
		};

		struct Status : Register<0x30, 32> { };

	public:

		/*
		 * This descriptor itself is stored in an uncached DMA page and
		 * 'dma_addr' is used to chain the descriptors together. The
		 * actual payload data is stored in the cached DMA buffer '_data'.
		 */

		class Descriptor : private Platform::Dma_buffer,
		                   private Genode::Mmio,
		                   public  Fifo<Descriptor>::Element
		{
			public:

				enum Drq        { SDRAM = 1, AUDIO_CODEC = 15 };
				enum Mode       { LINEAR = 0, IO = 1 };
				enum Block_size { EIGHT = 2 } ;
				enum Width      { BIT16 = 1, BIT32 = 2 };

			private:

				Platform::Dma_buffer _data;

				struct Config : Register<0x0, 32>
				{
					struct Src_drq        : Bitfield<0, 5> { };
					struct Src_mode       : Bitfield<5, 1> { };
					struct Src_block_size : Bitfield<6, 2> { };
					struct Src_data_width : Bitfield<9, 2> { };

					struct Dst_drq        : Bitfield<16, 5> { };
					struct Dst_mode       : Bitfield<21, 1> { };
					struct Dst_block_size : Bitfield<22, 2> { };
					struct Dst_data_width : Bitfield<25, 2> { };
				};

				/* 2 byte aligned */
				struct Src_phys_addr : Register<0x4, 32> { };
				struct Dst_phys_addr : Register<0x8, 32> { };

				struct Length : Register<0xc, 32> { };

				struct Parameter : Register<0x10, 32>
				{
					enum Mode { WAIT = 0, HANDSHAKE = 1 };
					struct Dst_mode : Bitfield<3, 1> { };
				};

				struct Next_phys_addr : Register<0x14, 32> { };

			public:

				Descriptor(Platform::Connection &platform, size_t size)
				: Platform::Dma_buffer(platform, 0x1000, UNCACHED),
				  Mmio(addr_t(local_addr<addr_t>())),
				  _data(platform, size, CACHED)
				{
					/* touch cached dma memory */
					touch_read((uint8_t *)data());

					write<Config::Src_block_size>(Block_size::EIGHT);
					write<Config::Dst_block_size>(Block_size::EIGHT);
					write<Parameter::Dst_mode>(Parameter::HANDSHAKE);

					dma_src(_data.dma_addr());
					dma_dst(_data.dma_addr());
					write<Length>(uint32_t(size));
				}

				void mode(Mode const src, Mode const dst)
				{
					write<Config::Src_mode>(src);
					write<Config::Dst_mode>(dst);
				}

				void drq(Drq const src, Drq const dst)
				{
					write<Config::Src_drq>(src);
					write<Config::Dst_drq>(dst);
				}

				void width(Width const src, Width const dst)
				{
					write<Config::Src_data_width>(src);
					write<Config::Dst_data_width>(dst);
				}

				void dma_src(addr_t const addr)  { write<Src_phys_addr>(uint32_t(addr));  }
				void dma_dst(addr_t const addr)  { write<Dst_phys_addr>(uint32_t(addr));  }
				void dma_next(addr_t const addr) { write<Next_phys_addr>(uint32_t(addr)); }

				void cache_maintainance()
				{
					if (read<Config::Src_drq>() == SDRAM)
						cache_clean_invalidate_data(data(), length());

					if (read<Config::Dst_drq>() == SDRAM)
						cache_invalidate_data(data(), length());
				}

				addr_t data_dma_addr() const { return _data.dma_addr(); }

				addr_t data()     const { return addr_t(_data.local_addr<addr_t>()); }
				addr_t dma_addr() const { return Dma_buffer::dma_addr();             }
				size_t length()   const { return read<Length>();                     }
		};

		class Channel : private Genode::Mmio
		{
			private:

				uint32_t const _id;
				Dma_engine    &_engine;

				Fifo<Descriptor> _queue { };

				struct Enable : Register<0x0, 32> { };
				struct Pause  : Register<0x4, 32> { };

				/* two byte aligend */
				struct Descr_phys_addr : Register<0x8, 32> { };

				struct Dma_cur_src   : Register<0x10, 32> { };
				struct Dma_cur_dest  : Register<0x14, 32> { };
				struct Dma_bcnt_left : Register<0x18, 32> { };

			public:

				Channel(addr_t const base, uint32_t const id, Dma_engine &engine)
				: Mmio(base), _id(id), _engine(engine)
				{ }

				enum Irq_type { HALF_PACKET = 1u, FULL_PACKET = 2u };

				void irq_enable(Irq_type const type)
				{
					Irq_enable::access_t irq = _engine.read<Irq_enable>();
					irq |= (type << (4 * _id));
					_engine.write<Irq_enable>(irq);
				}

				bool irq_pending(Irq_type const type)
				{
					Irq_pending::access_t pending = (_engine.read<Irq_pending>() >> (4 * _id)) & 0x7;
					if ((pending & type) == 0) return false;

					/* clear IRQs for engine */
					_engine.write<Irq_pending>(pending << (4 * _id));

					return true;
				}

				void enable()  { write<Enable>(1); };
				void disable() { write<Enable>(0); };

				void descr_dma(addr_t addr) { write<Descr_phys_addr>(uint32_t(addr)); }

				uint32_t cur_src()   const { return read<Dma_cur_src>(); }
				uint32_t cur_dest()  const { return read<Dma_cur_dest>(); }
				uint32_t bcnt_left() const { return read<Dma_bcnt_left>(); }

				/* queue handling */
				bool empty() const { return _queue.empty(); }

				void enqueue(Descriptor &descr)
				{
					descr.cache_maintainance();
					_queue.enqueue(descr);
				}

				template <typename FUNC>
				void dequeue(FUNC const &func) { _queue.dequeue(func); }

				template <typename FUNC>
				void head(FUNC const &func) { _queue.head(func); }
		};

	private:

		Constructible<Channel> _channel[8];

	public:

		Dma_engine(Platform::Device &device)
		: Mmio(device)
		{
			addr_t const channel_base = (addr_t)local_addr<addr_t>() + 0x100;
			for (uint32_t i = 0; i < 8; i++)
				_channel[i].construct(channel_base + i * 0x40, i, *this);

			/* disable auto gating for mclk */
			write<Auto_gate::Mclk_circuit>(1);

			/* make all 8 channels accessible from non-secure mode */
			write<Security::Channels>(0xff);
		}

		Channel &channel(uint32_t const id) { return *_channel[id]; }
};


struct Audio::Main
{
	Env &_env;

	Platform::Connection _platform { _env };

	Heap            _heap    { _env.ram(), _env.rm() };
	Audio::Session &_session { Session::construct(_env, _heap) };

	Platform::Device _device_audio { _platform, "audio_interface" };
	Platform::Device _device_dma   { _platform, "dma_controller" };

	Platform::Device::Irq _irq_audio { _device_audio };
	Platform::Device::Irq _irq_dma   { _device_dma };

	Signal_handler<Main> _irq_handler_audio { _env.ep(), *this,
		&Main::handle_audio_irq };
	Signal_handler<Main> _irq_handler_dma { _env.ep(), *this,
		&Main::handle_dma_irq };

	I2s_dma    _i2s_dma { _platform };
	I2s        _i2s     { _device_audio };
	Dma_engine _dma     { _device_dma };

	Dma_engine::Channel &_tx { _dma.channel(0) };
	Dma_engine::Channel &_rx { _dma.channel(1) };

	enum { TX = 2 };
	Constructible<Dma_engine::Descriptor> _tx_descr[TX];

	enum { RX = 2 };
	Constructible<Dma_engine::Descriptor> _rx_descr[RX];

	Main(Env &env) : _env(env)
	{
		_irq_audio.sigh(_irq_handler_audio);
		_irq_audio.ack();

		/* setup tx channel */
		for (unsigned i = 0; i < TX; i++)
			_tx_descr[i].construct(_platform, Session::Packet().size);

		for (unsigned i = 0; i < TX; i++) {
			/* cyclic descriptors (last points to first) */
			setup_tx_descriptor(*_tx_descr[i], _tx_descr[(i + 1) % TX]->dma_addr());
		}

		_tx.descr_dma(_tx_descr[0]->dma_addr());
		_tx.irq_enable(Dma_engine::Channel::FULL_PACKET);

		/* setup rx channel */
		for (unsigned i = 0; i < RX; i++)
			_rx_descr[i].construct(_platform, Session::Packet().size);

		for (unsigned i = 0; i < RX; i++) {
			/* cyclic descriptors (last points to first) */
			setup_rx_descriptor(*_rx_descr[i], _rx_descr[(i + 1) % RX]->dma_addr());
			_rx.enqueue(*_rx_descr[i]);
		}

		_rx.descr_dma(_rx_descr[0]->dma_addr());
		_rx.irq_enable(Dma_engine::Channel::FULL_PACKET);

		_irq_dma.sigh(_irq_handler_dma);
		_irq_dma.ack();

		tx();

		_tx.enable();
		_rx.enable();

		_i2s.enable();
	}

	void setup_tx_descriptor(Dma_engine::Descriptor &descr, addr_t const dma_addr_next)
	{
		using Descriptor = Dma_engine::Descriptor;

		descr.mode(Descriptor::LINEAR, Descriptor::IO);
		descr.drq(Descriptor::SDRAM, Descriptor::AUDIO_CODEC);
		descr.width(Descriptor::BIT32, Descriptor::BIT16);
		descr.dma_dst(_i2s_dma.tx_addr());
		descr.dma_next(dma_addr_next);
	}

	void setup_rx_descriptor(Dma_engine::Descriptor &descr, addr_t const dma_addr_next)
	{
		using Descriptor = Dma_engine::Descriptor;

		descr.mode(Descriptor::IO, Descriptor::LINEAR);
		descr.drq(Descriptor::AUDIO_CODEC, Descriptor::SDRAM);
		descr.width(Descriptor::BIT16, Descriptor::BIT32);
		descr.dma_src(_i2s_dma.rx_addr());
		descr.dma_next(dma_addr_next);
	}

	void fill(addr_t buffer)
	{
		Session::Packet packet = _session.play_packet();
		if (packet.valid())
			memcpy((void *)buffer, packet.data, packet.size);
		else
			memset((void *)buffer, 0, packet.size);
	}

	void tx()
	{
		if (_tx.empty()) {
			fill(_tx_descr[0]->data());
			/*
			 * Store in reverse so that the last descr is used first
			 * as the first one is currently played.
			 */
			for (int i = TX - 1; i >= 0; i--) {
				_tx.enqueue(*_tx_descr[i]);
			}
			return;
		}

		auto apply = [&](Dma_engine::Descriptor &descr)
		{
			fill(descr.data());
			_tx.enqueue(descr);
		};

		_tx.dequeue(apply);
	}

	void rx()
	{
		auto apply = [&](Dma_engine::Descriptor &descr)
		{
			Audio::Session::Packet packet { (int16_t *)descr.data(),
				descr.length() };
			_session.record_packet(packet);
			_rx.enqueue(descr);
		};

		_rx.dequeue(apply);
	}

	void handle_dma_irq()
	{
		if (_tx.irq_pending(Dma_engine::Channel::FULL_PACKET))

			/*
			 * This check is only necessary to cover any spurious
			 * interrupt that might occur.
			 */
			_tx.head([&] (Dma_engine::Descriptor &descr) {
				addr_t const cur = _tx.cur_src() & 0xfffff000u;
				if (cur != descr.data_dma_addr())
					tx();
			});

		if (_rx.irq_pending(Dma_engine::Channel::FULL_PACKET))
			rx();

		_irq_dma.ack();
	}

	/*
	 * This is just for debugging, since we have two cyclic DMA descriptors, this
	 * should not happen, it will only sound terrible ;)
	 */
	void handle_audio_irq()
	{
		if (_i2s.tx_underrun()) {
			warning("TX underrun");
		}
		if (_i2s.rx_overrun()) {
			warning("RX overrun");
		}
		_irq_audio.ack();
	}
};


void Component::construct(Genode::Env &env)
{
	static Audio::Main main(env);
}

