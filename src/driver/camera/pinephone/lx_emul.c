/*
 * \brief  PinePhone camera driver
 * \author Josef Soentgen
 * \date   2022-07-29
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul.h>
#include <lx_emul/alloc.h>

#include <linux/slab.h>


unsigned long emul_user_copy(void *to, void const *from, unsigned long n)
{
        memcpy(to, from, n);
        return 0;
}


#include <linux/sched.h>

void yield()
{
    lx_emul_task_schedule(false /* no block */);
}


#include <linux/fs.h>

int register_chrdev_region(dev_t from,unsigned count,const char * name)
{
	return 0;
}


#include <linux/fs.h>

int alloc_chrdev_region(dev_t * dev,unsigned baseminor,unsigned count,const char * name)
{
	/* use major from dynamic range */
	static dev_t counter = 254;
	*dev = MKDEV(counter--, baseminor);
	return 0;
}


#include <linux/cdev.h>


static int          cdevs_count;
static struct cdev *cdevs[6];

struct cdev *lx_emul_get_cdev(unsigned major, unsigned minor)
{
	dev_t dev = MKDEV(major, minor);
	size_t i;

	for (i = 0; i < sizeof (cdevs) / sizeof (cdevs[0]); i++) {
		if (cdevs[i] && cdevs[i]->dev == dev)
			return cdevs[i];
	}
	return NULL;
}


struct cdev *cdev_alloc(void)
{
	struct cdev *p = kzalloc(sizeof(struct cdev), GFP_KERNEL);
	return p;
}


void cdev_init(struct cdev * cdev,const struct file_operations * fops)
{
	cdev->ops = fops;
}


int cdev_add(struct cdev * p,dev_t dev,unsigned count)
{
	p->dev = dev;

	if (cdevs_count < sizeof (cdevs) / sizeof (cdevs[0])) {
		cdevs[cdevs_count] = p;
		++cdevs_count;
		return 0;
	}

	return -1;
}


int cdev_device_add(struct cdev * cdev,struct device * dev)
{
	return cdev_add(cdev, dev->devt, 1);
}


#include <linux/reset.h>

struct reset_control * __devm_reset_control_get(struct device * dev, const char * id,int index,bool shared,bool optional,bool acquired)
{
	return (struct reset_control*) id;
}


#include <linux/mmzone.h>

struct mem_section **mem_section;


#include <linux/dma-mapping.h>

/*
 * The CSI device is attached to the MBUS where the bus address differs
 * from the CPU's DMA adress. Normally 'drivers/soc/sunxi_mbus' will take
 * care of that but to keep things simply we shadow the 'dma_alloc_attrs'
 * function and apply the adjustment here.
 */

#define PHYS_OFFSET 0x40000000

int dma_mmap_attrs(struct device * dev,
                   struct vm_area_struct * vma,
                   void * cpu_addr,dma_addr_t dma_addr,
                   size_t size, unsigned long attrs)
{
	dma_addr_t adjust_dma_addr =
		dma_addr < PHYS_OFFSET ? dma_addr + PHYS_OFFSET
		                       : dma_addr;

	unsigned long addr = lx_emul_mem_virt_addr((void*)adjust_dma_addr);

	vma->vm_start = addr;
	vma->vm_end   = addr + size;

	return 0;
}


void * quirk_dma_alloc_attrs(struct device * dev,
                             size_t          size,
                             dma_addr_t    * dma_handle,
                             gfp_t           flag,
                             unsigned long   attrs)
{
	void * addr = dma_alloc_attrs(dev, size, dma_handle, flag, attrs);
	if (addr)
		*dma_handle = *dma_handle - PHYS_OFFSET;

	return addr;
}

#undef PHYS_OFFSET
