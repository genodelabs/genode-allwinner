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

namespace Sculpt { struct Network_section_dialog; }


struct Sculpt::Network_section_dialog : Registered<Section_dialog>
{
	void generate(Xml_generator &xml) const override
	{
		_gen_frame(xml, [&] {
			xml.attribute("style", "unimportant");
			gen_left_right(xml, 12,
				[&] {
					xml.node("label", [&] {
						xml.attribute("text", "Network");
						_gen_label_attr(xml);
					});
				},
				[&] {
					xml.node("label", [&] {
						xml.attribute("text", " ");
						_gen_status_attr(xml);
					});
				}
			);
		});
		if (detail == Detail::SELECTED) {
			xml.node("vbox", [&] { });
		}
	}

	Network_section_dialog(Registry<Registered<Section_dialog> > &dialogs)
	:
		Registered<Section_dialog>(dialogs, "network")
	{ }
};

#endif /* _VIEW__NETWORK_SECTION_DIALOG_H_ */
