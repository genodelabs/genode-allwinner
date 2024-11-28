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


#include <linux/blkdev.h>

int bd_prepare_to_claim(struct block_device * bdev,void * holder,
                        const struct blk_holder_ops *hops)
{
	struct block_device *whole = bdev_whole(bdev);
	whole->bd_claiming = holder;
	return 0;
}


void bd_abort_claiming(struct block_device * bdev,void * holder)
{
	struct block_device *whole = bdev_whole(bdev);
	whole->bd_claiming = NULL;
}


#include <linux/ioprio.h>

int __get_task_ioprio(struct task_struct * p)
{
	return IOPRIO_DEFAULT;
}
