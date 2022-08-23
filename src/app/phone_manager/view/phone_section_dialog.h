/*
 * \brief  Phone dialog
 * \author Norman Feske
 * \date   2022-05-20
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__PHONE_SECTION_DIALOG_H_
#define _VIEW__PHONE_SECTION_DIALOG_H_

#include <view/section_dialog.h>
#include <view/layout_helper.h>
#include <model/modem_state.h>
#include <model/current_call.h>

namespace Sculpt { struct Phone_section_dialog; }


struct Sculpt::Phone_section_dialog : Registered<Section_dialog>
{
	Modem_state const &_modem_state;

	void generate(Xml_generator &xml) const override
	{
		_gen_frame(xml, [&] {
			gen_left_right(xml, 12,
				[&] {
					xml.node("label", [&] {
						_gen_label_attr(xml);
						xml.attribute("text", "Phone");
					});
				},
				[&] {
					xml.node("label", [&] {
						_gen_status_attr(xml);

						if (!_modem_state.ready() || !_modem_state.pin_ok()) {
							_modem_state.gen_power_message(xml);
							return;
						}
						xml.attribute("text", "ready");
					});
				}
			);
		});
	}

	Phone_section_dialog(Registry<Registered<Section_dialog> > &dialogs,
	                     Modem_state const &modem_state)
	:
		Registered<Section_dialog>(dialogs, "phone"), _modem_state(modem_state)
	{ }
};

#endif /* _VIEW__PHONE_SECTION_DIALOG_H_ */
