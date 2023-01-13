/*
 * \brief  Dialog for the software status
 * \author Norman Feske
 * \date   2022-09-l9
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VIEW__SOFTWARE_STATUS_DIALOG_H_
#define _VIEW__SOFTWARE_STATUS_DIALOG_H_

#include <view/dialog.h>

namespace Sculpt { struct Software_status_dialog; }


struct Sculpt::Software_status_dialog
{
	Software_status const &_status;

	Software_status_dialog(Software_status const &status) : _status(status) { }

	void generate(Xml_generator &xml) const
	{
		gen_named_node(xml, "float", "software_status", [&] {
			_status.generate_software_status(xml); });
	}
};

#endif /* _VIEW__SOFTWARE_STATUS_DIALOG_H_ */
