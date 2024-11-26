/*
 * \brief  Dummy definitions of Linux Kernel functions - handled manually
 * \author Josef Soentgen
 * \date   2022-06-10
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul.h>


unsigned long net_rand_noise;


#include <asm/pgtable.h>

pgprot_t pgprot_writecombine(pgprot_t prot)
{
	lx_emul_trace_and_stop(__func__);
}


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


#include <drm/drm_cache.h>

void drm_memcpy_init_early(void)
{
    lx_emul_trace(__func__);
}


#include <linux/pinctrl/consumer.h>

struct pinctrl * devm_pinctrl_get(struct device * dev)
{
    lx_emul_trace(__func__);
	return (struct pinctrl*)ERR_PTR(-ENODEV);
}


#include <linux/pinctrl/consumer.h>

void devm_pinctrl_put(struct pinctrl * p)
{
    lx_emul_trace(__func__);
}


#include <linux/clk/clk-conf.h>

int of_clk_set_defaults(struct device_node * node,bool clk_supplier)
{
    lx_emul_trace(__func__);
	return 0;
}


#include <linux/reset.h>

struct reset_control * devm_reset_control_array_get(struct device * dev,bool shared,bool optional)
{
	// XXX check if devm_reset_control_array_get is needed
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/regulator/consumer.h>

struct regulator * devm_regulator_get_optional(struct device * dev,const char * id)
{
	lx_emul_trace(__func__);
	return (struct regulator*)ERR_PTR(-ENODEV);
}


#include <linux/sched.h>

void sched_set_fifo_low(struct task_struct * p)
{
	lx_emul_trace(__func__);
}


#include <linux/pinctrl/devinfo.h>

int pinctrl_init_done(struct device * dev)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/dma-mapping.h>

size_t dma_max_mapping_size(struct device * dev)
{
	lx_emul_trace(__func__);
	/* returning 0 will cap the size to UNIT_MAX */
	return 0;
}


#include <linux/swap.h>

void check_move_unevictable_pages(struct pagevec * pvec)
{
	lx_emul_trace(__func__);
}


#include <linux/mm.h>

bool is_vmalloc_addr(const void * x)
{
	lx_emul_trace(__func__);
	return false;
}


long strnlen_user(const char __user * str,long count)
{
	lx_emul_trace_and_stop(__func__);
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


#include <linux/pm_opp.h>

int devm_pm_opp_set_config(struct device * dev,struct dev_pm_opp_config * config)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/devfreq.h>

static struct devfreq _devfreq_dummy;

struct devfreq * devm_devfreq_add_device(struct device * dev,struct devfreq_dev_profile * profile,const char * governor_name,void * data)
{
	lx_emul_trace(__func__);
	return &_devfreq_dummy;
}


#include <linux/pm_opp.h>

void dev_pm_opp_put(struct dev_pm_opp * opp)
{
	lx_emul_trace(__func__);
}


#include <linux/pm_opp.h>

int dev_pm_opp_set_rate(struct device * dev,unsigned long target_freq)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/devfreq.h>

struct dev_pm_opp * devfreq_recommended_opp(struct device * dev,unsigned long * freq,u32 flags)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/devfreq.h>

void devm_devfreq_remove_device(struct device * dev,struct devfreq * devfreq)
{
	lx_emul_trace(__func__);
}


#include <linux/devfreq.h>

int devfreq_suspend_device(struct devfreq *devfreq)
{
	lx_emul_trace(__func__);
    return 0;
}


#include <linux/devfreq.h>

int devfreq_resume_device(struct devfreq *devfreq)
{
	lx_emul_trace(__func__);
    return 0;
}


#include <linux/pm_opp.h>

int devm_pm_opp_of_add_table(struct device * dev)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/swap.h>

void check_move_unevictable_folios(struct folio_batch * fbatch)
{
	lx_emul_trace(__func__);
}
