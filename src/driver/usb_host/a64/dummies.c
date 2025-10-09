/*
 * \brief  Dummy definitions of Linux Kernel functions
 * \author Sebastian Sumpf
 * \date   2022-07-13
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/prandom.h>

DEFINE_PER_CPU(unsigned long, net_rand_noise);
EXPORT_PER_CPU_SYMBOL(net_rand_noise);


#include <linux/sysfs.h>

void sysfs_remove_bin_file(struct kobject * kobj,const struct bin_attribute * attr)
{
	lx_emul_trace(__func__);
}


#include <linux/sysfs.h>

void sysfs_remove_file_from_group(struct kobject * kobj,const struct attribute * attr,const char * group)
{
	lx_emul_trace(__func__);
}


#include <linux/sysfs.h>

void sysfs_unmerge_group(struct kobject * kobj,const struct attribute_group * grp)
{
	lx_emul_trace(__func__);
}


#include <linux/fs.h>

int __register_chrdev(unsigned int major,unsigned int baseminor,unsigned int count,const char * name,const struct file_operations * fops)
{
	lx_emul_trace(__func__);
	return 0;
}


int register_chrdev_region(dev_t from,unsigned count,const char * name)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/cdev.h>

int cdev_add(struct cdev * p,dev_t dev,unsigned count)
{
	lx_emul_trace(__func__);
	return 0;
}


void cdev_del(struct cdev * p)
{
	lx_emul_trace(__func__);
}


#include <linux/kernfs.h>

struct kernfs_node * kernfs_find_and_get_ns(struct kernfs_node * parent,const char * name,const void * ns)
{
	lx_emul_trace(__func__);
	static struct kernfs_node node;
	return &node;
}


void kernfs_notify(struct kernfs_node * kn)
{
	lx_emul_trace(__func__);
}


#include <linux/proc_fs.h>

struct proc_dir_entry { int dummy; };

struct proc_dir_entry * proc_create_seq_private(const char * name,umode_t mode,struct proc_dir_entry * parent,const struct seq_operations * ops,unsigned int state_size,void * data)
{
	static struct proc_dir_entry ret;
	lx_emul_trace(__func__);
	return &ret;
}


#include <linux/pinctrl/devinfo.h>

int pinctrl_bind_pins(struct device * dev)
{
	lx_emul_trace(__func__);
	return 0;
}


int pinctrl_init_done(struct device * dev)
{
	lx_emul_trace(__func__);
	return 0;
}




#include <linux/clk/clk-conf.h>

int of_clk_set_defaults(struct device_node * node,bool clk_supplier)
{
	return 0;
}


#include <linux/clk.h>

struct clk * of_clk_get(struct device_node * np,int index)
{
	return NULL;
}



struct clk * devm_clk_get(struct device * dev,const char * id)
{
	return NULL;
}


#include <linux/reset.h>

struct reset_control * __devm_reset_control_get(struct device * dev,const char * id,int index,bool shared,bool optional,bool acquired)
{
	return NULL;
}


struct reset_control * devm_reset_control_array_get(struct device * dev,bool shared,bool optional)
{
	return NULL;
}


int reset_control_deassert(struct reset_control * rstc)
{
	return 0;
}


#include <linux/gpio/consumer.h>

struct gpio_desc * __must_check devm_gpiod_get_optional(struct device * dev,const char * con_id,enum gpiod_flags flags)
{
	lx_emul_trace(__func__);
	return NULL;
}


int gpiod_to_irq(const struct gpio_desc * desc)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/regulator/consumer.h>

struct regulator * regulator_get_optional(struct device * dev,const char * id)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/extcon-provider.h>

struct extcon_dev * devm_extcon_dev_allocate(struct device * dev,const unsigned int * supported_cable)
{
	lx_emul_trace(__func__);
	return NULL;
}


int devm_extcon_dev_register(struct device * dev,struct extcon_dev * edev)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/regulator/consumer.h>

struct regulator * devm_regulator_get_optional(struct device * dev,const char * id)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/mm.h>

bool is_vmalloc_addr(const void * x)
{
	lx_emul_trace(__func__);
	return false;
}


#include <linux/fs.h>

struct timespec64 current_time(struct inode * inode)
{
	struct timespec64 ret = { 0 };
	lx_emul_trace(__func__);
	return ret;
}


#include <linux/pid.h>

void put_pid(struct pid * pid)
{
	lx_emul_trace(__func__);
}


#include <linux/cred.h>

void __put_cred(struct cred * cred)
{
	lx_emul_trace(__func__);
}


#include <linux/thread_info.h>

void __check_object_size(const void * ptr,unsigned long n,bool to_user)
{
	lx_emul_trace(__func__);
}


#include <linux/skbuff.h>

void __init skb_init(void)
{
	lx_emul_trace(__func__);
}


#include <net/net_namespace.h>

void __init net_ns_init(void)
{
	lx_emul_trace(__func__);
}


#include <linux/clk.h>

struct clk * devm_clk_get_optional(struct device * dev,const char * id)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/cdev.h>

void cdev_init(struct cdev * cdev,const struct file_operations * fops)
{
	lx_emul_trace(__func__);
}
