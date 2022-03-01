/*
 * \brief  Pinephone audio codec routing
 * \author Sebastian Sumpf
 * \date   2022-03-11
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _AUDIO_CODEC_H_
#define _AUDIO_CODEC_H_

#include <platform_session/device.h>

namespace Audio {
	using namespace Genode;
	class Codec;
	class Analog_domain;
	class Analog;
	class Device;
}

class Audio::Codec : Platform::Device::Mmio
{
	private:

		/**
		 * DA
		 */
		struct I2s_ap_format : Register<0x4, 32> { };

		struct I2s_ap_fifo   : Register<0x14, 32>
		{
			struct Rxom : Bitfield<0, 2> { };
			struct Txim : Bitfield<2, 1> { };
		};

		struct I2s_ap_clock_divide : Register<0x24, 32>
		{
			struct Mclkdiv  : Bitfield<0, 4> { };
			struct Bclkdiv  : Bitfield<4, 3> { };
			struct Mclko_en : Bitfield<7, 1> { };
		};

		struct I2s_ap_tx_counter : Register<0x28, 32> { };
		struct I2s_ap_rx_counter : Register<0x2c, 32> { };

		/**
		 * Codec
		 */
		struct System_clock_control : Register<0x20c, 32>
		{
			struct Sysclk_src     : Bitfield<0, 1>  { enum { AIF2 =  1 }; };
			struct Sysclk_enable  : Bitfield<3, 1>  { };
			struct Aif2clk_src    : Bitfield<4, 2>  { enum { AUDIO_PLL = 2 }; };
			struct Aif2clk_enable : Bitfield<7, 1>  { };
			struct Aif1clk_src    : Bitfield<8, 2>  { enum { AUDIO_PLL = 1 }; };
			struct Aif1clk_enable : Bitfield<11, 1> { };
		};

		struct Module_clock_control : Register<0x210, 32>
		{
			struct Dac  : Bitfield<2, 1>  { };
			struct Adc  : Bitfield<3, 1>  { };
			struct Aif2 : Bitfield<14, 1> { };
			struct Aif1 : Bitfield<15, 1> { };
		};

		struct Module_reset_control : Register<0x214, 32>
		{
			struct Dac  : Bitfield<2, 1>  { };
			struct Adc  : Bitfield<3, 1>  { };
			struct Aif2 : Bitfield<14, 1> { };
			struct Aif1 : Bitfield<15, 1> { };
		};

		struct System_sample_rate : Register<0x218, 32> { };

		struct Aif1_clock_control : Register<0x240, 32>
		{
			struct Word_size : Bitfield<4, 2> { };
			struct Lrck_div  : Bitfield<6, 3>  { };
			struct Bclk_div  : Bitfield<9, 4>  { };
			struct Master    : Bitfield<15, 1> { };
		};

		struct Aif1_dacdat_control : Register<0x248, 32>
		{
			struct Dac0r_src : Bitfield<8, 2> { };
			struct Dac0l_src : Bitfield<10, 2> { };
		};

		struct Aif1_digital_mixer_source : Register<0x24c, 32>
		{
			struct Adc0r_src : Bitfield<8, 4> { };
			struct Adc0l_src : Bitfield<12, 4> { };
		};

		struct Aif2_clock_control : Register<0x280, 32>
		{
			struct Data_fmt  : Bitfield<2, 2>  { enum { DSP      = 0x3 }; };
			struct Word_size : Bitfield<4, 2>  { enum { _16BIT   = 1   }; };
			struct Lrck_div  : Bitfield<6, 3>  { enum { DIV32    = 1   }; };
			struct Bclk_div  : Bitfield<9, 4>  { enum { DIV96    = 0xb }; };
			struct Bclk_inv  : Bitfield<14, 1> { enum { INVERTED = 1   }; };
			struct Master    : Bitfield<15, 1> { enum { MASTER   = 0   }; };
		};

		struct Aif2_adcdat_control : Register<0x284, 32>
		{
			struct Right_enable : Bitfield<14, 1> { };
			struct Left_enable  : Bitfield<15, 1> { };
			struct Loop : Bitfield<0, 1> { };
		};

		struct Aif2_dacdat_control : Register<0x288, 32>
		{
			enum { MIX_MONO = 0x3 }; /* (left + right) / 2 */
			struct Dacr_src    : Bitfield<8, 2>  { };
			struct Dacl_src    : Bitfield<10, 2> { };
			struct Dacl_enable : Bitfield<15, 1> { };
		};

		struct Aif2_digital_mixer_source : Register<0x28c, 32>
		{
			struct Adcr_scr : Bitfield<8, 4>  { };
			struct Adcl_scr : Bitfield<12, 4> { };
		};

		struct Adc_digital : Register<0x300, 32>
		{
			struct Enad : Bitfield<15, 1> { };
		};

		struct Hmic_control_1 : Register<0x310, 32>
		{
			struct Jack_in_irq_en           : Bitfield<4, 1> { };
			struct Mdata_threshold_debounce : Bitfield<5, 2> { };
		};

		struct Hmic_control_2 : Register<0x314, 32>
		{
			struct Sf            : Bitfield<6, 2>  { enum { X1X2 = 1 }; };
			struct Sample_select : Bitfield<14, 2> { enum { DOWN_BY_TWO = 1 }; };
		};

		struct Dac_control : Register<0x320, 32>
		{
			struct Enable : Bitfield<15, 1> { };
		};

		struct Dac_mixer_source : Register<0x330, 32>
		{
			struct Dacr_aif2_dacr : Bitfield<9, 1>  { };
			struct Dacl_aif2_dacl : Bitfield<13, 1> { };
		};

		/* set defaults = modem from/to ADC/DAC path */
		void _init()
		{
			/* system clock */
			using S = System_clock_control;
			write<S::Sysclk_src>(S::Sysclk_src::AIF2);
			write<S::Sysclk_enable>(1);
			write<S::Aif2clk_src>(S::Aif2clk_src::AUDIO_PLL);
			write<S::Aif2clk_enable>(1);

			write<Module_reset_control::Dac>(1);
			write<Module_reset_control::Adc>(1);
			write<Module_reset_control::Aif2>(1);

			write<System_sample_rate>(0); /* 8 KHz */

			/* AIF2 goes to modem */
			using A = Aif2_clock_control;
			write<A::Data_fmt>(A::Data_fmt::DSP);
			write<A::Word_size>(A::Word_size::_16BIT);
			write<A::Lrck_div>(A::Lrck_div::DIV32);
			write<A::Bclk_div>(A::Bclk_div::DIV96);
			write<A::Bclk_inv>(A::Bclk_inv::INVERTED);
			write<A::Master>(A::Master::MASTER);

			/* enables clock */
			write<Module_clock_control::Aif2>(1);
			write<Module_clock_control::Dac>(1);
			write<Module_clock_control::Adc>(1);

			/* ADC enable */
			write<Aif2_adcdat_control::Right_enable>(1);
			write<Aif2_adcdat_control::Left_enable>(1);

			/* DAC sources */
			write<Aif2_dacdat_control::Dacr_src>(Aif2_dacdat_control::MIX_MONO);
			write<Aif2_dacdat_control::Dacl_src>(Aif2_dacdat_control::MIX_MONO);
			write<Aif2_dacdat_control::Dacl_enable>(1);

			/* AIF2 mixer source = ADC (mic) */
			write<Aif2_digital_mixer_source::Adcr_scr>(1);
			write<Aif2_digital_mixer_source::Adcl_scr>(1);

			/* ADC Digital part enable */
			write<Adc_digital::Enad>(1);

			/* DAC mixer is AIF2_DAC = modem */
			write<Dac_mixer_source::Dacr_aif2_dacr>(1);
			write<Dac_mixer_source::Dacl_aif2_dacl>(1);

			/* enable digital part */
			write<Dac_control::Enable>(1);

			write<Hmic_control_1::Jack_in_irq_en>(1);
			write<Hmic_control_1::Mdata_threshold_debounce>(1);

			using H = Hmic_control_2;
			write<H::Sf>(H::Sf::X1X2);
			write<H::Sample_select>(H::Sample_select::DOWN_BY_TWO);
		}

	public:

		Codec(Platform::Device &device)
		: Mmio(device)
		{ _init(); }
};


class Audio::Analog_domain : Platform::Device::Mmio
{
	private:

		struct Ac_pr : Register<0x0, 32>
		{
			struct Out  : Bitfield<0, 8>  { };
			struct In   : Bitfield<8, 8>  { };
			struct Addr : Bitfield<16, 5> { };
			struct Rw   : Bitfield<24, 1> { };
			struct Rst  : Bitfield<28, 1> { };
		};

	public:

		Analog_domain(Platform::Device &device)
		: Mmio(device)
		{ }

		void write(uint8_t opcode, uint8_t data)
		{
			using Mmio = Platform::Device::Mmio;
			Mmio::write<Ac_pr::Rst>(1);
			Mmio::write<Ac_pr::Addr>(opcode);
			Mmio::write<Ac_pr::In>(data);
			Mmio::write<Ac_pr::Rw>(1);
			Mmio::write<Ac_pr::Rw>(0);
		}

		uint8_t read(uint8_t opcode)
		{
			using Mmio = Platform::Device::Mmio;
			Mmio::write<Ac_pr::Rst>(1);
			Mmio::write<Ac_pr::Rw>(0);
			Mmio::write<Ac_pr::Addr>(opcode);
			return Mmio::read<Ac_pr::Out>();
		}
};


class Audio::Analog
{
	private:

		Analog_domain _analog;

		template <uint8_t _OPCODE>
		struct Command : Genode::Register<8>
		{
			enum { OPCODE = _OPCODE };
		};

		template <class COMMAND>
		void write(uint8_t data)
		{
			_analog.write(COMMAND::OPCODE, data);
		}

		template <class COMMAND>
		uint8_t read()
		{
			return _analog.read(COMMAND::OPCODE);
		}

		/**************
		 ** Commands **
		 **************/

		struct Output_mixer_left : Command<0x1>
		{
			struct Dac_mute_left : Bitfield<1, 1> { };
		};

		Output_mixer_left::access_t _out_mixer_left { 0 };

		struct Output_mixer_right : Command<0x2>
		{
			struct Dac_mute_right : Bitfield<1, 1> { };
		};

		Output_mixer_right::access_t _out_mixer_right { 0 };

		struct Earpiece_control0 : Command<0x3>
		{
			struct Input_src : Bitfield<0, 2> { enum { DACL = 1 }; };
		};

		Earpiece_control0::access_t _earpiece0 { 0 };

		struct Earpiece_control1 : Command<0x4>
		{
			struct Enable_pa : Bitfield<7, 1> { };
			struct Mute_off  : Bitfield<6, 1> { };
			struct Volume    : Bitfield<0, 5> { }; /* mute: 0 and 1 */
		};

		Earpiece_control1::access_t _earpiece1 { 0 };

		struct Lineout_control0 : Command<0x5>
		{
			struct Right_src : Bitfield<4, 1> { enum { MONO_DIFF = 1  }; };
			struct Left_src  : Bitfield<5, 1> { enum { LEFT_RIGHT = 1 }; };
			struct Enable    : Bitfield<6, 2> { enum { LEFT_RIGHT = 3 }; };
		};

		Lineout_control0::access_t _lineout0 { 0 };

		struct Lineout_control1 : Command<0x6>
		{
			struct Volume : Bitfield<0, 5> { }; /* mute: 0 and 1 */
		};

		Lineout_control1::access_t _lineout1 { 0 };

		struct Mic1_control : Command<0x7>
		{
			struct Boost            : Bitfield<0, 3> { };
			struct Boost_amp_enable : Bitfield<3, 1> { };
			struct Gain             : Bitfield<4, 3> { };
		};

		Mic1_control::access_t _mic1 { 0x34 };

		struct Dac_mixer : Command<0xa>
		{
			enum { LEFT_RIGHT = 0x3 };
			struct Dac_enable   : Bitfield<6, 2> { };
			struct Mixer_enable : Bitfield<4, 2> { };
		};

		Dac_mixer::access_t _dac_mixer { 0 };

		struct Adc_mixer_left : Command<0xb>
		{
			struct Mic1 : Bitfield<6, 1> { };
		};

		Adc_mixer_left::access_t _adc_left { 0 };

		struct Adc_mixer_right : Command<0xc>
		{
			struct Mic1 : Bitfield<6, 1> { };
		};

		Adc_mixer_left::access_t _adc_right { 0 };

		struct Adc : Command<0xd>
		{
			struct Input_gain    : Bitfield<0, 3> { };
			struct Left_enable   : Bitfield<6, 1> { };
			struct Right_enable  : Bitfield<7, 1> { };
		};

		Adc::access_t _adc = { 0x3 };

		struct Hs_mbias_control : Command<0xe>
		{
			struct Master_mic_enable : Bitfield<7, 1> { };
		};

		Hs_mbias_control::access_t _hs_mbias { 0x21 };

	public:

		void enable_mic1()
		{
			Mic1_control::Boost_amp_enable::set(_mic1, 1);
			write<Mic1_control>(_mic1);

			Adc_mixer_left::Mic1::set(_adc_left, 1);
			write<Adc_mixer_left>(_adc_left);

			Adc_mixer_right::Mic1::set(_adc_right, 1);
			write<Adc_mixer_right>(_adc_right);

			Adc::Left_enable::set(_adc, 1);
			Adc::Right_enable::set(_adc, 1);
			write<Adc>(_adc);

			Hs_mbias_control::Master_mic_enable::set(_hs_mbias, 1);
			write<Hs_mbias_control>(_hs_mbias);
		}

		void enable_earpiece()
		{
			using E0 = Earpiece_control0;
			E0::Input_src::set(_earpiece0, E0::Input_src::DACL);
			write<E0>(_earpiece0);

			Earpiece_control1::Enable_pa::set(_earpiece1, 1);
			Earpiece_control1::Mute_off::set(_earpiece1,  1);
			Earpiece_control1::Volume::set(_earpiece1, 0x1f); /* maximum volume */
			write<Earpiece_control1>(_earpiece1);

			Dac_mixer::Dac_enable::set(_dac_mixer, Dac_mixer::LEFT_RIGHT);
			Dac_mixer::Mixer_enable::set(_dac_mixer, Dac_mixer::LEFT_RIGHT);
			write<Dac_mixer>(_dac_mixer);
		}

		void enable_speaker()
		{
			Lineout_control1::Volume::set(_lineout1, 0x1f); /* maximum volume */
			write<Lineout_control1>(_lineout1);

			using L0 = Lineout_control0;
			L0::Right_src::set(_lineout0, L0::Right_src::MONO_DIFF);
			L0::Left_src::set(_lineout0, L0::Left_src::LEFT_RIGHT);
			L0::Enable::set(_lineout0, L0::Enable::LEFT_RIGHT);
			write<L0>(_lineout0);

			Output_mixer_left::Dac_mute_left::set(_out_mixer_left, 1);
			Output_mixer_right::Dac_mute_right::set(_out_mixer_right, 1);
			write<Output_mixer_left>(_out_mixer_left);
			write<Output_mixer_right>(_out_mixer_right);

			Dac_mixer::Dac_enable::set(_dac_mixer, Dac_mixer::LEFT_RIGHT);
			Dac_mixer::Mixer_enable::set(_dac_mixer, Dac_mixer::LEFT_RIGHT);
			write<Dac_mixer>(_dac_mixer);
		}

		Analog(Platform::Device &device)
		: _analog(device)
		{
			enable_mic1();
			enable_earpiece();
			//enable_speaker();
		}
};


class Audio::Device
{
	private:

		Env &_env;

		Platform::Connection _platform { _env };
		Platform::Device     _device_codec  { _platform, "audio_codec" };
		Platform::Device     _device_analog { _platform, "audio_analog" };
		Codec                _codec { _device_codec };
		Analog               _analog { _device_analog };

	public:

		Device(Env &env)
		: _env(env)
		{ }
};

#endif /* _AUDIO_CODEC_H_ */
