/*
 * \brief  Allwinner EMAC Ethernet driver
 * \author Norman Feske
 * \date   2021-06-02
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <linux/mmzone.h>

struct mem_section **mem_section;


#include <linux/reset.h>

struct reset_control { };

static struct reset_control _reset_control;

struct reset_control * __devm_reset_control_get(struct device * dev,const char * id,int index,bool shared,bool optional,bool acquired)
{
	return &_reset_control;
}


#include <linux/gfp.h>
#include <linux/slab.h>

unsigned long get_zeroed_page(gfp_t gfp_mask)
{
	return (unsigned long)__alloc_pages(GFP_KERNEL, 0, 0, NULL)->virtual;
}


#include <linux/mm.h>

bool is_vmalloc_addr(const void * x)
{
	return false;
}


#include <linux/gfp.h>

void * page_frag_alloc_align(struct page_frag_cache * nc, unsigned int fragsz,
                             gfp_t gfp_mask, unsigned int align_mask)
{
	if (align_mask != ~0U) {
		printk("page_frag_alloc_align: unsupported align_mask=%x\n", align_mask);
		lx_emul_trace_and_stop(__func__);
	}
	return lx_emul_mem_alloc_aligned(fragsz, ARCH_KMALLOC_MINALIGN);
}


void page_frag_free(void * addr)
{
	lx_emul_mem_free(addr);
}


#include <linux/proc_fs.h>

struct proc_dir_entry { int dummy; };

struct proc_dir_entry * proc_create_seq_private(const char * name,umode_t mode,struct proc_dir_entry * parent,const struct seq_operations * ops,unsigned int state_size,void * data)
{
	static struct proc_dir_entry ret;
	lx_emul_trace(__func__);
	return &ret;
}


#include <linux/regulator/consumer.h>

struct regulator * devm_regulator_get_optional(struct device * dev,const char * id)
{
	return ERR_PTR(-ENODEV);
}


#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <../drivers/regulator/internal.h>

struct regulator * devm_regulator_get(struct device * dev,const char * id)
{
	static struct regulator dummy = { };
	return &dummy;
}


#include <linux/interrupt.h>

void do_softirq_own_stack(void)
{
    /*
     * We have no IRQ stack to switch to anyway,
     * so we stay here in contrast to the original
     * implementation
     */
    __do_softirq();
}
