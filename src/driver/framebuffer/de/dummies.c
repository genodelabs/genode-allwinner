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


#include <linux/fb.h>

struct apertures_struct;

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


int fb_deferred_io_init(struct fb_info *info)
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


#include <net/net_namespace.h>

void __init net_ns_init(void)
{
	lx_emul_trace(__func__);
}


#include <linux/skbuff.h>

void __init skb_init(void)
{
	lx_emul_trace(__func__);
}


#include <linux/mm.h>

bool is_vmalloc_addr(const void * x)
{
	lx_emul_trace(__func__);
	return false;
}


#include <drm/drm_aperture.h>

int drm_aperture_remove_conflicting_framebuffers(resource_size_t base, resource_size_t size,
                                                 const struct drm_driver *req_driver)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <video/nomodeset.h>

bool video_firmware_drivers_only(void)
{
	lx_emul_trace(__func__);
	return false;
}


#include <video/cmdline.h>

const char * video_get_options(const char * name)
{
	static char * const option = "";
	lx_emul_trace(__func__);
	return option;
}


#include <linux/regulator/consumer.h>

struct regulator * devm_regulator_get_optional(struct device * dev,const char * id)
{
	lx_emul_trace(__func__);
	return NULL;
}
