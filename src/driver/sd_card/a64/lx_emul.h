/*
 * \brief  Allwinner A64 driver Linux MMC/SD port
 * \author Josef Soentgen
 * \date   2022-07-18
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */


/* Needed to trace and stop */
#include <lx_emul/debug.h>

/* __sched include */
#include <linux/sched/debug.h>
