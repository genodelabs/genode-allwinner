/*
 * \brief  Interface to obtain software-related diagnostic information
 * \author Norman Feske
 * \date   2022-01-13
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _SOFTWARE_STATUS_H_
#define _SOFTWARE_STATUS_H_

#include <types.h>

namespace Sculpt { struct Software_status; }


struct Sculpt::Software_status : Interface
{
	virtual bool    software_status_available() const = 0;
	virtual void    generate_software_status(Xml_generator &) const = 0;
};

#endif /* _SOFTWARE_STATUS_H_ */
