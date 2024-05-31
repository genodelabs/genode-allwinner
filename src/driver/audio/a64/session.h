/*
 * \brief  Audio session bindings
 * \author Sebastian Sumpf
 * \date   2022-08-27
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#ifndef _SESSION_H_
#define _SESSION_H_

#include <base/allocator.h>
#include <audio_in_session/audio_in_session.h>
#include <audio_out_session/audio_out_session.h>

namespace Audio {
	struct Session;

	/* make sure the periods match as 'Packet' is used by both */
	static_assert(Audio_out::PERIOD == Audio_in::PERIOD);
}

struct Audio::Session
{
	struct Packet
	{
		Genode::int16_t *data { nullptr };
		Genode::size_t   size { Audio_out::PERIOD * sizeof(short) * 2 };

		bool valid() const { return data != nullptr; }
	};

	virtual Packet play_packet(void) = 0;
	virtual void record_packet(Packet) = 0;
	virtual ~Session() { }

	static Session &construct(Genode::Env &env, Genode::Allocator &alloc);
};

#endif /* _SESSION_H_ */


