/*
 * \brief  Modem-configuration handling of the AT-protocol driver
 * \author Norman Feske
 * \date   2022-08-10
 *
 * Certain modem settings (QCFG) such as the usbnet mode are maintained
 * persistently by the modem. Hence the modem's current mode of operation
 * cannot be assumed at startup but must be queried.
 *
 * Each 'Qcfg::Entry' corresponds to one setting that the modem driver relies
 * on. The AT protocol driver maintains a registry of QCFG configuration
 * entries, queries the state of each entry at initialization time, and applies
 * a configuration change if needed. The progress is tracked via the
 * 'Qcfg::Entry::state' member variables.
 *
 * If any setting changed, the modem must be rebooted in order to make the
 * new configuration effective.
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _AT_PROTOCOL_QCFG_H_
#define _AT_PROTOCOL_QCFG_H_

/* Genode includes */
#include <base/registry.h>

/* AT-protocol includes */
#include <at_protocol/types.h>

namespace At_protocol { struct Qcfg; }


class At_protocol::Qcfg : Noncopyable
{
	public:

		using Name  = String<32>;
		using Value = String<32>;

		struct Entry : Noncopyable, private Registry<Entry>::Element
		{
			Name  const name;
			Value const value;

			enum class State {
				UNKNOWN,    /* current modem configuration still uncertain */
				CONFIRMED,  /* is already consistent with 'value' */
				MISMATCH,   /* differs from 'value', 'Assign_qcfg' in progress */
				MODIFIED    /* got changed to 'value', reboot needed */
			};

			State state = State::UNKNOWN;

			Entry(Qcfg &qcfg, Name const &name, Value const &value)
			:
				Registry<Entry>::Element(qcfg._entries, *this), name(name), value(value)
			{ }
		};

	private:

		Registry<Entry> _entries { };

		template <typename FN>
		void _with_any_entry_in_state(Entry::State state, FN const &fn) const
		{
			bool done = false;
			_entries.for_each([&] (Entry const &e) {
				if (!done && (e.state == state)) {
					fn(e);
					done = true;
				}
			});
		}

	public:

		/*
		 * Used during the querying of the current settings
		 */
		template <typename FN>
		void with_any_unknown_entry(FN const &fn) const
		{
			_with_any_entry_in_state(Entry::State::UNKNOWN, fn);
		}

		template <typename FN>
		void with_any_mismatching_entry(FN const &fn) const
		{
			_with_any_entry_in_state(Entry::State::MISMATCH, fn);
		}

		/*
		 * Used to handle the responses of 'Query_qcfg' or 'Assign_qcfg'
		 */
		template <typename FN>
		void with_entry(Name const &name, FN const &fn)
		{
			bool done = false;
			_entries.for_each([&] (Entry &e) {
				if (!done && (e.name == name)) {
					fn(e);
					done = true;
				}
			});
		}

		bool reboot_needed() const
		{
			bool any_unknown = false, any_mismatch = false, any_modified = false;

			_entries.for_each([&] (Entry const &e) {
				any_unknown  |= (e.state == Entry::State::UNKNOWN);
				any_mismatch |= (e.state == Entry::State::MISMATCH);
				any_modified |= (e.state == Entry::State::MODIFIED);
			});

			/* don't reboot while querying or assigning values */
			if (any_unknown || any_mismatch)
				return false;

			return any_modified;
		}

		/*
		 * Force new querying of settings after reboot
		 */
		void invalidate_after_reboot()
		{
			_entries.for_each([&] (Entry &e) {
				e.state = Entry::State::UNKNOWN; });
		}
};

#endif /* _AT_PROTOCOL_QCFG_H_ */
