/*
 * \brief  Software-management dialog
 * \author Norman Feske
 * \date   2022-05-20
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__SOFTWARE_SECTION_DIALOG_H_
#define _VIEW__SOFTWARE_SECTION_DIALOG_H_

#include <view/section_dialog.h>
#include <view/layout_helper.h>
#include <model/software_status.h>

namespace Sculpt { struct Software_section_dialog; }


struct Sculpt::Software_section_dialog : Registered<Section_dialog>
{
	Software_status const &_status;

	void generate(Xml_generator &xml) const override
	{
		_gen_frame(xml, [&] {
			gen_left_right(xml, 12,
				[&] {
					xml.node("label", [&] {
						xml.attribute("text", "Software");
						_gen_label_attr(xml);
					});
				},
				[&] {
					xml.node("label", [&] {
						xml.attribute("text", _status.software_status_message());
						_gen_status_attr(xml);
					});
				}
			);
		});
	}

	Software_section_dialog(Registry<Registered<Section_dialog> > &dialogs,
	                        Software_status const &status)
	:
		Registered<Section_dialog>(dialogs, "software"),
		_status(status)
	{ }
};

#endif /* _VIEW__SOFTWARE_SECTION_DIALOG_H_ */
