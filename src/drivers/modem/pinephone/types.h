/*
 * \brief  Types used by the modem driver
 * \author Norman Feske
 * \date   2022-06-15
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _TYPES_H_
#define _TYPES_H_

#include <util/xml_generator.h>

namespace Modem {

	using namespace Genode;

	struct Delayer;
}


struct Modem::Delayer : Interface
{
	virtual void msleep(unsigned long) = 0;
};

#endif /* _TYPES_H_ */
