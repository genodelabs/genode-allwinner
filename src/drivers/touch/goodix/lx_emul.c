/*
 * \brief  Linux emulation environment for Goodix touchscreen driver
 * \author Norman Feske
 * \date   2021-06-02
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/fs.h>

int alloc_chrdev_region(dev_t * dev,unsigned baseminor,unsigned count,const char * name)
{
	static dev_t counter = 0;
	*dev = counter++;
	return 0;
}


#include <linux/ioport.h>

struct resource * __request_region(struct resource * parent,resource_size_t start,resource_size_t n,const char * name,int flags)
{
	static struct resource ret;

	lx_emul_trace(__func__);

	/* called by of_io_request_and_map, return value is only checked against 0 */
	return &ret;
}


#include <linux/sched.h>

long io_schedule_timeout(long timeout)
{
	return schedule_timeout(timeout);
}


#include <linux/random.h>

void add_interrupt_randomness(int irq,int irq_flags)
{
	lx_emul_trace(__func__);
}


#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/slab.h>

int simple_pin_fs(struct file_system_type * type, struct vfsmount ** mount, int * count)
{
	*mount = kmalloc(sizeof(struct vfsmount), GFP_KERNEL);
	return 0;
}


#include <linux/fs.h>

void simple_release_fs(struct vfsmount ** mount,int * count)
{
	kfree(*mount);
}


#include <linux/fs.h>

struct inode * alloc_anon_inode(struct super_block * s)
{
	return kmalloc(sizeof(struct inode), GFP_KERNEL);
}


#include <linux/atomic.h>
#include <linux/dma-fence.h>

u64 dma_fence_context_alloc(unsigned num)
{
	static atomic64_t dma_fence_context_counter = ATOMIC64_INIT(1);
	WARN_ON(!num);
	return atomic64_add_return(num, &dma_fence_context_counter) - num;
}


#include <linux/fb.h>

int fb_get_options(const char * name,char ** option)
{
	*option = "";
	return 0;
}


#include <linux/shmem_fs.h>

struct file * shmem_file_setup(const char * name,loff_t size,unsigned long flags)
{
	static struct file file;
	lx_emul_trace(__func__);
	return &file;
}


#include <linux/firmware.h>

int request_firmware_nowait(struct module * module,bool uevent,const char * name,
                            struct device * device,gfp_t gfp,void * context,
                            void (* cont)(const struct firmware * fw,void * context))
{
	/* call 'goodix_config_cb' to finish device initialization */
	cont(NULL, context);
	return 0;
}


#include <linux/clk/clk-conf.h>

int of_clk_set_defaults(struct device_node * node,bool clk_supplier)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <../drivers/regulator/internal.h>

struct regulator * devm_regulator_get(struct device * dev,const char * id)
{
	static struct regulator dummy = { };

	/*
	 * Artificially defer the probing of the touchscreen driver once
	 * to prevent the driver from using the i2c bus before the i2c driver
	 * is initialized.
	 */
	{
		static bool deferred_once = false;
		if (!deferred_once) {
			deferred_once = true;
			return ERR_PTR(-EPROBE_DEFER);
		}
	}

	return &dummy;
}


#include <linux/regulator/consumer.h>

int regulator_enable(struct regulator * regulator)
{
	lx_emul_trace(__func__);
	return 0;
}


