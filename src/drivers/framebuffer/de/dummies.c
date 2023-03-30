/*
 * \brief  Dummy definitions of Linux Kernel functions - handled manually
 * \author Norman Feske
 * \date   2021-07-22
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul.h>


#include <linux/prandom.h>

DEFINE_PER_CPU(unsigned long, net_rand_noise);
EXPORT_PER_CPU_SYMBOL(net_rand_noise);


#include <linux/kernfs.h>

struct kernfs_node * kernfs_find_and_get_ns(struct kernfs_node * parent,const char * name,const void * ns)
{
	return NULL;
}


#include <linux/random.h>

int __must_check get_random_bytes_arch(void * buf,int nbytes)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/proc_fs.h>

struct proc_dir_entry { int dummy; };

struct proc_dir_entry * proc_create_seq_private(const char * name,umode_t mode,struct proc_dir_entry * parent,const struct seq_operations * ops,unsigned int state_size,void * data)
{
	static struct proc_dir_entry ret;
	lx_emul_trace(__func__);
	return &ret;
}


#include <linux/fs.h>

int __register_chrdev(unsigned int major, unsigned int baseminor,unsigned int count,const char * name,const struct file_operations * fops)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/random.h>

void get_random_bytes(void * buf,int nbytes)
{
	lx_emul_trace(__func__);
}


#include <asm/pgtable.h>

pgprot_t pgprot_writecombine(pgprot_t prot)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/srcu.h>

void call_srcu(struct srcu_struct * ssp,struct rcu_head * rhp,rcu_callback_t func)
{
	lx_emul_trace(__func__);
}


#include <linux/cdev.h>

void cdev_init(struct cdev * cdev,const struct file_operations * fops)
{
	lx_emul_trace(__func__);
}


#include <linux/cdev.h>

int cdev_device_add(struct cdev * cdev,struct device * dev)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/cdev.h>

void cdev_device_del(struct cdev * cdev,struct device * dev)
{
	lx_emul_trace(__func__);
}


#include <linux/sched.h>

void sched_set_fifo(struct task_struct * p)
{
	printk("sched_set_fifo called, doing nothing\n");
}


#include <linux/fb.h>

int remove_conflicting_framebuffers(struct apertures_struct * a,const char * name,bool primary)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/kobject.h>

int kobject_uevent_env(struct kobject * kobj,enum kobject_action action,char * envp_ext[])
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/dma-resv.h>

void dma_resv_init(struct dma_resv * obj)
{
	lx_emul_trace(__func__);
}


#include <linux/fb.h>

void fb_set_suspend(struct fb_info *info, int state)
{
	lx_emul_trace_and_stop(__func__);
}


void framebuffer_release(struct fb_info *info)
{
	lx_emul_trace_and_stop(__func__);
}


void unregister_framebuffer(struct fb_info *fb_info)
{
	lx_emul_trace_and_stop(__func__);
}


void fb_deferred_io_cleanup(struct fb_info *info)
{
	lx_emul_trace_and_stop(__func__);
}


void sys_fillrect(struct fb_info *info, const struct fb_fillrect *rect)
{
	lx_emul_trace_and_stop(__func__);
}


void sys_copyarea(struct fb_info *info, const struct fb_copyarea *area)
{
	lx_emul_trace_and_stop(__func__);
}


void sys_imageblit(struct fb_info *info, const struct fb_image *image)
{
	lx_emul_trace_and_stop(__func__);
}


void cfb_fillrect(struct fb_info *info, const struct fb_fillrect *rect)
{
	lx_emul_trace_and_stop(__func__);
}


void cfb_copyarea(struct fb_info *info, const struct fb_copyarea *area)
{
	lx_emul_trace_and_stop(__func__);
}


void cfb_imageblit(struct fb_info *info, const struct fb_image *image)
{
	lx_emul_trace_and_stop(__func__);
}


void fb_deferred_io_init(struct fb_info *info)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/regulator/consumer.h>

int regulator_bulk_register_supply_alias(struct device * dev,const char * const * id,
                                         struct device * alias_dev,const char * const * alias_id,int num_id)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/clk/clk-conf.h>

int of_clk_set_defaults(struct device_node * node,bool clk_supplier)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/wait_bit.h>

void __init wait_bit_init(void)
{
	lx_emul_trace(__func__);
}
