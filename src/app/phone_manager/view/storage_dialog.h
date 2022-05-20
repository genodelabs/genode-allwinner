/*
 * \brief  Storage-management dialog
 * \author Norman Feske
 * \date   2022-05-20
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__STORAGE_DIALOG_H_
#define _VIEW__STORAGE_DIALOG_H_

#include <view/section_dialog.h>
#include <view/layout_helper.h>

namespace Sculpt { struct Storage_dialog; }


struct Sculpt::Storage_dialog : Registered<Section_dialog>
{
	void generate(Xml_generator &xml) const override
	{
		_gen_frame(xml, [&] {
			gen_left_right(xml, 12,
				[&] {
					xml.node("label", [&] {
						xml.attribute("text", "Storage");
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
	}

	Storage_dialog(Registry<Registered<Section_dialog> > &dialogs)
	:
		Registered<Section_dialog>(dialogs, "storage")
	{ }
};

#endif /* _VIEW__STORAGE_DIALOG_H_ */
