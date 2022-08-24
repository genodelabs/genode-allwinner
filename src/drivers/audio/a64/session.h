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

namespace Audio {
	struct Session;
}

struct Audio::Session
{
	struct Packet
	{
		Genode::int16_t *data { nullptr };
		Genode::size_t   size { 2048 };

		bool valid() const { return data != nullptr; }
	};

	virtual Packet play_packet(void) = 0;
	virtual void record_packet(Packet) = 0;
	virtual ~Session() { }

	static Session &construct(Genode::Env &env, Genode::Allocator &alloc);
};

#endif /* _SESSION_H_ */


