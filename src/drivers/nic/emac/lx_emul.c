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
	return (unsigned long)kzalloc(PAGE_SIZE, gfp_mask | __GFP_ZERO);
}


#include <linux/slab.h>

struct kmem_cache * kmem_cache_create_usercopy(const char * name,
                                               unsigned int size,
                                               unsigned int align,
                                               slab_flags_t flags,
                                               unsigned int useroffset,
                                               unsigned int usersize,
                                               void (* ctor)(void *))
{
	return kmem_cache_create(name, size, align, flags, ctor);
}


#include <linux/slab.h>

int kmem_cache_alloc_bulk(struct kmem_cache * s,gfp_t flags, size_t nr,void ** p)
{
	size_t i;
	for (i = 0; i < nr; i++)
		p[i] = kmem_cache_alloc(s, flags);

	return nr;
}


#include <linux/mm.h>

bool is_vmalloc_addr(const void * x)
{
	return false;
}


#include <linux/dma-mapping.h>

dma_addr_t dma_map_page_attrs(struct device * dev, struct page * page,size_t offset,
                              size_t size, enum dma_data_direction dir, unsigned long attrs)
{
	dma_addr_t    const dma_addr  = page_to_phys(page);
	unsigned long const virt_addr = (unsigned long)page_to_virt(page);

	lx_emul_mem_cache_clean_invalidate((void *)(virt_addr + offset), size);

	return dma_addr + offset;
}


void dma_unmap_page_attrs(struct device * dev, dma_addr_t addr, size_t size,
                          enum dma_data_direction dir, unsigned long attrs)
{
	/*
	 * In principle, this function should call 'lx_emul_mem_cache_invalidate'
	 * to invalidate the cache for DMA buffers before being used for receiving
	 * network packets. However, the access pattern by the EMAC NIC driver
	 * apparently does not require it.
	 */
}


#include <linux/random.h>

int __must_check get_random_bytes_arch(void * buf,int nbytes)
{
	printk("get_random_bytes_arch: leaving buffer unmodified\n");
	return 0;
}


void get_random_bytes(void * buf,int nbytes)
{
	printk("get_random_bytes: leaving buffer unmodified\n");
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


#include <linux/sysfs.h>

int sysfs_create_dir_ns(struct kobject * kobj,const void * ns)
{
	lx_emul_trace(__func__);
	kobj->sd = kzalloc(sizeof(*kobj->sd), GFP_KERNEL);
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


