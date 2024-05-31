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

#include <lx_emul.h>
#include <linux/slab.h>


#include <linux/fs.h>

int alloc_chrdev_region(dev_t * dev,unsigned baseminor,unsigned count,const char * name)
{
	static dev_t counter = 0;
	*dev = counter++;
	return 0;
}


#include <asm/cache.h>

extern int cache_line_size(void);
int cache_line_size(void)
{
	return ARCH_DMA_MINALIGN;
}


#include <linux/regulator/consumer.h>

static int vmmc_regulator;

struct regulator * devm_regulator_get_optional(struct device * dev,const char * id)
{
	if (strcmp(id, "vqmmc") == 0)
		return ERR_PTR(-ENOSYS);

	if (strcmp(id, "vmmc") == 0)
		return (struct regulator*)&vmmc_regulator;

	return NULL;
}


int regulator_get_voltage(struct regulator * regulator)
{
	if (regulator == (struct regulator *)&vmmc_regulator)
		return 3300000;

	return 0;
}


#include <linux/slab.h>

void * kmem_cache_alloc_lru(struct kmem_cache * cachep,struct list_lru * lru,gfp_t flags)
{
	return kmem_cache_alloc(cachep, flags);
}


#include <linux/ioprio.h>

int __get_task_ioprio(struct task_struct * p)
{
	return IOPRIO_DEFAULT;
}
