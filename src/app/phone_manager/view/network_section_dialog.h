/*
 * \brief  Network dialog
 * \author Norman Feske
 * \date   2022-05-20
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__NETWORK_SECTION_DIALOG_H_
#define _VIEW__NETWORK_SECTION_DIALOG_H_

#include <view/section_dialog.h>
#include <view/layout_helper.h>
#include <model/nic_target.h>
#include <model/nic_state.h>

namespace Sculpt { struct Network_section_dialog; }


struct Sculpt::Network_section_dialog : Registered<Section_dialog>
{
	Nic_target const &_nic_target;
	Nic_state  const &_nic_state;

	void generate(Xml_generator &xml) const override
	{
		_gen_frame(xml, [&] {
			gen_left_right(xml, 12,
				[&] {
					xml.node("label", [&] {
						xml.attribute("text", "Network");
						_gen_label_attr(xml);
					});
				},
				[&] {
					xml.node("label", [&] {
						_gen_status_attr(xml);

						auto status_message = [&]
						{
							switch (_nic_target.type()) {
							case Nic_target::Type::UNDEFINED:
							case Nic_target::Type::OFF:
								break;

							case Nic_target::Type::DISCONNECTED:
								return "disconnected";

							case Nic_target::Type::WIRED:
								return _nic_state.ready() ? "LAN" : "LAN ...";

							case Nic_target::Type::WIFI:
								return _nic_state.ready() ? "WLAN" : "WLAN ...";

							case Nic_target::Type::MODEM:
								return _nic_state.ready() ? "mobile" : "mobile ...";
							}
							return "off";
						};
						xml.attribute("text", status_message());
					});
				}
			);
		});
		if (detail == Detail::SELECTED) {
			xml.node("vbox", [&] { });
		}
	}

	Network_section_dialog(Registry<Registered<Section_dialog> > &dialogs,
	                       Nic_target const &nic_target,
	                       Nic_state  const &nic_state)
	:
		Registered<Section_dialog>(dialogs, "network"),
		_nic_target(nic_target), _nic_state(nic_state)
	{ }
};

#endif /* _VIEW__NETWORK_SECTION_DIALOG_H_ */
