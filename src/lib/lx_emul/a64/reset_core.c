/*
 * \brief  Linux kernel / Lx_emul reset pin control
 * \author Stefan Kalkowski
 * \date   2021-05-25
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/reset.h>
#include <lx_emul/reset.h>

struct reset_control * __devm_reset_control_get(struct device * dev,
                                                const char    * id,
                                                int             index,
                                                bool            shared,
                                                bool            optional,
                                                bool            acquired)
{
	return (struct reset_control*) id;
}


int reset_control_deassert(struct reset_control * rstc)
{
	lx_emul_reset_deassert((const char *)rstc);
	return 0;
}
