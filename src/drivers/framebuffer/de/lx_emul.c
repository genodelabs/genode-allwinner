/*
 * \brief  Allwinner framebuffer driver
 * \author Norman Feske
 * \date   2021-06-02
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/sched.h>

int __cond_resched(void)
{
	if (should_resched(0)) {
		schedule();
		return 1;
	}
	return 0;
}


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


#include <linux/dma-mapping.h>

int dma_supported(struct device *dev, u64 mask)
{
	return 1;
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


#include <linux/sched/wake_q.h>

void wake_q_add(struct wake_q_head * head,struct task_struct * task)
{
	lx_emul_trace(__func__);
	wake_up_process(task);
}