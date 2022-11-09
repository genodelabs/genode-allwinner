/*
 * \brief  Device dialog
 * \author Norman Feske
 * \date   2022-05-20
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__DEVICE_SECTION_DIALOG_H_
#define _VIEW__DEVICE_SECTION_DIALOG_H_

#include <model/power_state.h>
#include <view/section_dialog.h>
#include <view/layout_helper.h>
#include <view/conditional_float_dialog.h>


namespace Sculpt { struct Device_section_dialog; }


struct Sculpt::Device_section_dialog : Registered<Section_dialog>
{
	Power_state const &_power_state;

	void generate(Xml_generator &xml) const override
	{
		_gen_frame(xml, [&] {
			gen_left_right(xml, 12,
				[&] {
					xml.node("label", [&] {
						xml.attribute("text", "Device");
						_gen_label_attr(xml);
					});
				},
				[&] {
					xml.node("label", [&] {
						xml.attribute("text", _power_state.summary());
						_gen_status_attr(xml);
					});
				}
			);
		});
	}

	Device_section_dialog(Registry<Registered<Section_dialog> > &dialogs,
	                      Power_state const &power_state)
	:
		Registered<Section_dialog>(dialogs, "device"),
		_power_state(power_state)
	{ }
};

#endif /* _VIEW__DEVICE_SECTION_DIALOG_H_ */
