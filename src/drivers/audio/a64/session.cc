/*
 * \brief  Genode Audio out/in session (legacy)
 * \author Sebastian Sumpf
 * \date   2022-08-26
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <audio_in_session/rpc_object.h>
#include <audio_out_session/rpc_object.h>
#include <play_session/connection.h>
#include <record_session/connection.h>
#include <base/attached_rom_dataspace.h>
#include <base/session_label.h>
#include <base/component.h>
#include <base/heap.h>
#include <root/component.h>

#include "session.h"

using namespace Genode;

namespace Audio_out {
	class  Session_component;
	class  Out;
	class  Root;
	struct Root_policy;
	enum   Channel_number { LEFT, RIGHT, MAX_CHANNELS, INVALID = MAX_CHANNELS };
	static Session_component *channel_acquired[MAX_CHANNELS];
}

/**************
 ** Playback **
 **************/

class Audio_out::Session_component : public Audio_out::Session_rpc_object
{
	private:

		Channel_number _channel;

	public:

		Session_component(Genode::Env &env, Channel_number channel, Signal_context_capability cap)
		:
			Session_rpc_object(env, cap), _channel(channel)
		{
			Audio_out::channel_acquired[_channel] = this;
		}

		~Session_component()
		{
			Audio_out::channel_acquired[_channel] = 0;
		}
};


class Audio_out::Out
{
	private:

		Genode::Env                            &_env;
		Genode::Signal_handler<Audio_out::Out>  _data_avail_dispatcher;

		Stream *left()  { return channel_acquired[LEFT]->stream(); }
		Stream *right() { return channel_acquired[RIGHT]->stream(); }

		void _advance_position(Packet *l, Packet *r)
		{
			bool full_left = left()->full();
			bool full_right = right()->full();

			left()->pos(left()->packet_position(l));
			right()->pos(right()->packet_position(r));

			left()->increment_position();
			right()->increment_position();

			Session_component *channel_left  = channel_acquired[LEFT];
			Session_component *channel_right = channel_acquired[RIGHT];

			if (full_left)
				channel_left->alloc_submit();

			if (full_right)
				channel_right->alloc_submit();
		}

		void _handle_data_avail() { }


	public:

		Out(Genode::Env &env)
		:
			_env(env),
			_data_avail_dispatcher(env.ep(), *this, &Audio_out::Out::_handle_data_avail)
		{ }

		static bool channel_number(const char     *name,
		                           Channel_number *out_number)
		{
			static struct Names {
				const char     *name;
				Channel_number  number;
			} names[] = {
				{ "left", LEFT }, { "front left", LEFT },
				{ "right", RIGHT }, { "front right", RIGHT },
				{ 0, INVALID }
			};

			for (Names *n = names; n->name; ++n)
				if (!Genode::strcmp(name, n->name)) {
					*out_number = n->number;
					return true;
				}

			return false;
		}

		Signal_context_capability data_avail() { return _data_avail_dispatcher; }

		Audio::Session::Packet play_packet()
		{
			Audio::Session::Packet packet { };

			unsigned lpos = left()->pos();
			unsigned rpos = right()->pos();

			Packet *p_left  = left()->get(lpos);
			Packet *p_right = right()->get(rpos);

			if (p_left->valid() && p_right->valid()) {
				/* convert float to S16LE */
				static short data[Audio_out::PERIOD * Audio_out::MAX_CHANNELS];

				for (unsigned i = 0; i < Audio_out::PERIOD * Audio_out::MAX_CHANNELS; i += 2) {
					data[i] = int16_t(p_left->content()[i / 2] * 32767);
					data[i + 1] = int16_t(p_right->content()[i / 2] * 32767);
				}

				packet.data  = data;
				packet.size  = sizeof(data);

				p_left->invalidate();
				p_right->invalidate();

				p_left->mark_as_played();
				p_right->mark_as_played();
			} else {
				return packet;
			}

			_advance_position(p_left, p_right);

			/* always report when a period has passed */
			Session_component *channel_left  = channel_acquired[LEFT];
			Session_component *channel_right = channel_acquired[RIGHT];
			channel_left->progress_submit();
			channel_right->progress_submit();

			return packet;
		}
};


/**
 * Session creation policy for our service
 */
struct Audio_out::Root_policy
{
	void aquire(const char *args)
	{
		size_t ram_quota =
			Arg_string::find_arg(args, "ram_quota"  ).ulong_value(0);
		size_t session_size =
			align_addr(sizeof(Audio_out::Session_component), 12);

		if ((ram_quota < session_size) ||
		    (sizeof(Stream) > ram_quota - session_size)) {
			Genode::error("insufficient 'ram_quota', got ", ram_quota,
			              " need ", sizeof(Stream) + session_size);
			throw Genode::Insufficient_ram_quota();
		}

		char channel_name[16];
		Channel_number channel_number;
		Arg_string::find_arg(args, "channel").string(channel_name,
		                                             sizeof(channel_name),
		                                             "left");
		if (!Out::channel_number(channel_name, &channel_number)) {
			Genode::error("invalid output channel '",(char const *)channel_name,"' requested, "
			              "denying '",Genode::label_from_args(args),"'");
			throw Genode::Service_denied();
		}
		if (Audio_out::channel_acquired[channel_number]) {
			Genode::error("output channel '",(char const *)channel_name,"' is unavailable, "
			              "denying '",Genode::label_from_args(args),"'");
			throw Genode::Service_denied();
		}
	}

	void release() { }
};


namespace Audio_out {
	typedef Root_component<Session_component, Root_policy> Root_component;
}


/**
 * Root component, handling new session requests.
 */
class Audio_out::Root : public Audio_out::Root_component
{
	private:

		Genode::Env &_env;

		Signal_context_capability _cap;

	protected:

		Session_component *_create_session(const char *args) override
		{
			char channel_name[16];
			Channel_number channel_number = INVALID;
			Arg_string::find_arg(args, "channel").string(channel_name,
			                                             sizeof(channel_name),
			                                             "left");
			Out::channel_number(channel_name, &channel_number);

			return new (md_alloc())
				Session_component(_env, channel_number, _cap);
		}

	public:

		Root(Genode::Env &env, Allocator &md_alloc,
		     Signal_context_capability cap)
		:
			Root_component(env.ep(), md_alloc),
			_env(env), _cap(cap)
		{ }
};


/***************
 ** Recording **
 ***************/

namespace Audio_in {

	class Session_component;
	class In;
	class Root;
	struct Root_policy;
	static Session_component *channel_acquired;
	enum Channel_number { LEFT, MAX_CHANNELS, INVALID = MAX_CHANNELS };

}


class Audio_in::Session_component : public Audio_in::Session_rpc_object
{
	private:

		Channel_number _channel;

	public:

		Session_component(Genode::Env &env, Channel_number channel)
		: Session_rpc_object(env, Signal_context_capability()),
		  _channel(channel)
		{ channel_acquired = this; }

		~Session_component() { channel_acquired = nullptr; }
};


class Audio_in::In
{
	private:

		bool _active() { return channel_acquired && channel_acquired->active(); }

		Stream *stream() { return channel_acquired->stream(); }

	public:

		static bool channel_number(const char     *name,
		                           Channel_number *out_number)
		{
			static struct Names {
				const char     *name;
				Channel_number  number;
			} names[] = {
				{ "left", LEFT },
				{ 0, INVALID }
			};

			for (Names *n = names; n->name; ++n)
				if (!Genode::strcmp(name, n->name)) {
					*out_number = n->number;
					return true;
				}

			return false;
		}

		void record_packet(Audio::Session::Packet &packet)
		{
			if (!_active()) return;
			/*
			 * Check for an overrun first and notify the client later.
			 */
			bool overrun = stream()->overrun();

			Packet *p = stream()->alloc();

			float const scale = 32768.0f * 2;

			float * const content = p->content();
			short * const data    = packet.data;
			memset(content, 0, Audio_in::PERIOD * sizeof(float));

			for (unsigned long i = 0; i < packet.size/sizeof(short); i += 2) {
				float sample = data[i] + data[i+1];
				content[i/2] = sample / scale;
			}

			stream()->submit(p);

			channel_acquired->progress_submit();

			if (overrun) channel_acquired->overrun_submit();
		}
};


struct Audio_in::Root_policy
{
	void aquire(char const *args)
	{
		size_t ram_quota = Arg_string::find_arg(args, "ram_quota").ulong_value(0);
		size_t session_size = align_addr(sizeof(Audio_in::Session_component), 12);

		if ((ram_quota < session_size) ||
		    (sizeof(Stream) > (ram_quota - session_size))) {
			Genode::error("insufficient 'ram_quota', got ", ram_quota,
			              " need ", sizeof(Stream) + session_size,
			              ", denying '",Genode::label_from_args(args),"'");
			throw Genode::Insufficient_ram_quota();
		}

		char channel_name[16];
		Channel_number channel_number;
		Arg_string::find_arg(args, "channel").string(channel_name,
		                                             sizeof(channel_name),
		                                             "left");
		if (!In::channel_number(channel_name, &channel_number)) {
			Genode::error("invalid input channel '",(char const *)channel_name,"' requested, "
			              "denying '",Genode::label_from_args(args),"'");
			throw Genode::Service_denied();
		}
		if (Audio_in::channel_acquired) {
			Genode::error("input channel '",(char const *)channel_name,"' is unavailable, "
			              "denying '",Genode::label_from_args(args),"'");
			throw Genode::Service_denied();
		}
	}

	void release() { }
};


namespace Audio_in {
	typedef Root_component<Session_component, Root_policy> Root_component;
}


/**
 * Root component, handling new session requests.
 */
class Audio_in::Root : public Audio_in::Root_component
{
	private:

		Genode::Env               &_env;

	protected:

		Session_component *_create_session(char const *args) override
		{
			char channel_name[16];
			Channel_number channel_number = INVALID;
			Arg_string::find_arg(args, "channel").string(channel_name,
			                                             sizeof(channel_name),
			                                             "left");
			In::channel_number(channel_name, &channel_number);
			return new (md_alloc()) Session_component(_env, channel_number);
		}

	public:

		Root(Genode::Env &env, Allocator &md_alloc)
		: Root_component(env.ep(), md_alloc), _env(env) { }
};


struct Audio_aggregator : Audio::Session
{
	Env       &env;
	Allocator &alloc;

	Audio_out::Out  out { env };
	Audio_out::Root out_root { env, alloc, out.data_avail() };

	Audio_in::In   in { };
	Audio_in::Root in_root { env, alloc };

	Audio_aggregator(Env &env, Allocator &alloc)
	: env(env), alloc(alloc)
	{
		env.parent().announce(env.ep().manage(out_root));
		env.parent().announce(env.ep().manage(in_root));
	}

	bool _audio_out_active()
	{
		using namespace Audio_out;
		return  channel_acquired[LEFT] && channel_acquired[RIGHT] &&
	        channel_acquired[LEFT]->active() && channel_acquired[RIGHT]->active();
	}

	Packet play_packet(void) override
	{
		return _audio_out_active() ? out.play_packet() : Packet();
	}

	void record_packet(Packet packet) override
	{
		in.record_packet(packet);
	}
};


struct Record_play_aggregator : Audio::Session
{
	static constexpr unsigned SAMPLES_PER_PERIOD = Audio_in::PERIOD;
	static constexpr unsigned CHANNELS = 2;

	Env &_env;

	struct Stereo_output : private Noncopyable
	{
		Env &_env;

		/* 16 bit per sample, interleaved left and right */
		int16_t data[SAMPLES_PER_PERIOD*CHANNELS] { };

		Record::Connection _left  { _env, "left"  };
		Record::Connection _right { _env, "right" };

		Stereo_output(Env &env) : _env(env) { }

		void clear() { for (auto &e : data) e = 0; }

		void from_record_sessions()
		{
			using Samples_ptr = Record::Connection::Samples_ptr;

			Record::Num_samples const num_samples { SAMPLES_PER_PERIOD };

			auto clamped = [&] (float v)
			{
				return (v >  1.0) ?  1.0
				     : (v < -1.0) ? -1.0
				     :  v;
			};

			auto float_to_s16 = [&] (float v) { return int16_t(clamped(v)*32767); };

			_left.record(num_samples,
				[&] (Record::Time_window const tw, Samples_ptr const &samples) {

					for (unsigned i = 0; i < SAMPLES_PER_PERIOD; i++)
						data[i*CHANNELS] = float_to_s16(samples.start[i]);

					_right.record_at(tw, num_samples,
						[&] (Samples_ptr const &samples) {
							for (unsigned i = 0; i < SAMPLES_PER_PERIOD; i++)
								data[i*CHANNELS + 1] = float_to_s16(samples.start[i]);
						});
				},
				[&] { clear(); }
			);
		}
	};

	Stereo_output _stereo_output { _env };

	struct Stereo_input : private Noncopyable
	{
		struct Frame { float left, right; };

		void _for_each_frame(Packet const &packet, auto const &fn) const
		{
			float const scale = 1.0f/32768;

			for (unsigned i = 0; i < SAMPLES_PER_PERIOD; i++)
				fn(Frame { .left  = scale*float(packet.data[i*CHANNELS]),
				           .right = scale*float(packet.data[i*CHANNELS + 1]) });
		}

		Env &_env;

		Play::Connection _left  { _env, "mic_left"  };
		Play::Connection _right { _env, "mic_right" };

		Play::Time_window _time_window { };

		Stereo_input(Env &env) : _env(env) { }

		void from_packet(Packet const &packet)
		{
			if (!packet.valid())
				return;

			Play::Duration const duration_us { 11*1000 };
			_time_window = _left.schedule_and_enqueue(_time_window, duration_us,
				[&] (auto &submit) {
					_for_each_frame(packet, [&] (Frame const frame) {
						submit(frame.left); }); });

			_right.enqueue(_time_window,
				[&] (auto &submit) {
					_for_each_frame(packet, [&] (Frame const frame) {
						submit(frame.right); }); });
		}
	};

	Stereo_input _stereo_input { _env };

	Record_play_aggregator(Env &env) : _env(env) { }

	Packet play_packet() override
	{
		_stereo_output.from_record_sessions();

		return { _stereo_output.data, sizeof(Stereo_output::data) };
	}

	void record_packet(Packet packet) override
	{
		_stereo_input.from_packet(packet);
	}
};


Audio::Session &Audio::Session::construct(Env &env, Allocator &alloc)
{
	bool const use_record_play_interface =
		Attached_rom_dataspace(env, "config").xml().attribute_value("record_play", false);

	if (!use_record_play_interface) {
		static Audio_aggregator _audio { env, alloc };
		return _audio;
	}

	static Record_play_aggregator _audio { env };
	return _audio;
}
