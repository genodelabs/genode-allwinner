/*
 * \brief  Dummy definitions of Linux Kernel functions
 * \author Norman Feske
 * \date   2021-06-04
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul.h>


#include <linux/random.h>

void add_interrupt_randomness(int irq,int irq_flags)
{
	lx_emul_trace(__func__);
}


#include <asm/memory.h>

/* definition in arch/arm64/mm/mmu.c */
u64 kimage_voffset;

/* definition in mm/init.c */
s64 memstart_addr = -1; /* referenced by PHYS_OFFSET, presumably not accessed */


#include <linux/prandom.h>

DEFINE_PER_CPU(unsigned long, net_rand_noise);
EXPORT_PER_CPU_SYMBOL(net_rand_noise);


#include <linux/filter.h>
#include <linux/jump_label.h> /* for DEFINE_STATIC_KEY_FALSE */

void bpf_prog_change_xdp(struct bpf_prog *prev_prog, struct bpf_prog *prog)
{
	lx_emul_trace_and_stop(__func__);
}

DEFINE_STATIC_KEY_FALSE(bpf_stats_enabled_key);


#include <net/sock.h>

DEFINE_STATIC_KEY_FALSE(memalloc_socks_key);


#include <linux/rcutree.h>

void synchronize_rcu_expedited(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pinctrl/devinfo.h>

int pinctrl_bind_pins(struct device * dev)
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


/*
 * Hook useful for replacing drivers/mfd/syscon.c
 *
 * #include <linux/mfd/syscon.h>
 *
 * struct regmap * syscon_regmap_lookup_by_phandle(struct device_node * np,const char * property)
 * {
 *   lx_emul_trace(__func__);
 *   return (struct regmap *)0x550;
 * }
 */


#include <linux/pinctrl/devinfo.h>

int pinctrl_init_done(struct device * dev)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/reset.h>

int reset_control_assert(struct reset_control * rstc)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/reset.h>

int reset_control_deassert(struct reset_control * rstc)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/netdevice.h>

void netdev_rss_key_fill(void * buffer,size_t len)
{
	printk("netdev_rss_key_fill leaves buffer unmodified\n");
	lx_emul_trace(__func__);
}


#include <linux/rtnetlink.h>

void rtnl_lock(void)
{
	lx_emul_trace(__func__);
}


#include <linux/rtnetlink.h>

void rtnl_unlock(void)
{
	lx_emul_trace(__func__);
}


#include <linux/gpio/consumer.h>

struct gpio_desc * __must_check
devm_gpiod_get_optional(struct device * dev, const char * con_id, enum gpiod_flags flags)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/gpio/consumer.h>

struct gpio_desc * __must_check
gpiod_get_optional(struct device * dev, const char * con_id, enum gpiod_flags flags)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/gpio/consumer.h>

void gpiod_set_value_cansleep(struct gpio_desc * desc,int value)
{
	lx_emul_trace(__func__);
}


#include <linux/reset.h>

struct reset_control * __reset_control_get(struct device * dev,const char * id,int index,bool shared,bool optional,bool acquired)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/rtnetlink.h>

int rtnl_lock_killable(void)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/rtnetlink.h>

int rtnl_is_locked(void)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <net/rtnetlink.h>

void rtnl_register(int protocol,int msgtype,rtnl_doit_func doit,rtnl_dumpit_func dumpit,unsigned int flags)
{
	lx_emul_trace(__func__);
}


#include <linux/gfp.h>

void free_pages(unsigned long addr,unsigned int order)
{
	printk("free_pages leaks 2^%d pages\n", order);
}


#include <linux/stringhash.h>

unsigned int full_name_hash(const void * salt,const char * name,unsigned int len)
{
	printk("full_name_hash returns 0\n");
	return 0;
}


#include <linux/random.h>

void add_device_randomness(const void * buf,unsigned int size)
{
	lx_emul_trace(__func__);
}


#include <linux/srcu.h>

void call_srcu(struct srcu_struct * ssp,struct rcu_head * rhp,rcu_callback_t func)
{
	lx_emul_trace(__func__);
}


#include <linux/rcupdate.h>

void synchronize_rcu(void)
{
	lx_emul_trace(__func__);
}
#include <linux/async.h>

void async_synchronize_full(void)
{
	lx_emul_trace(__func__);
}


#include <linux/dma-mapping.h>

void dma_sync_single_for_cpu(struct device * dev,dma_addr_t addr,size_t size,enum dma_data_direction dir)
{
	lx_emul_trace(__func__);
}


#include <linux/dma-mapping.h>

void dma_sync_single_for_device(struct device * dev,dma_addr_t addr,size_t size,enum dma_data_direction dir)
{
	lx_emul_trace(__func__);
}


#include <linux/inetdevice.h>

int devinet_ioctl(struct net * net,unsigned int cmd,struct ifreq * ifr)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <net/route.h>

int ip_rt_ioctl(struct net * net,unsigned int cmd,struct rtentry * rt)
{
	lx_emul_trace(__func__);
	return 0;
}


extern int __of_attach_node_sysfs(struct device_node * np);
int __of_attach_node_sysfs(struct device_node * np)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/proc_ns.h>

int proc_alloc_inum(unsigned int * inum)
{
	*inum = 1; /* according to linux/proc_ns.h without CONFIG_PROC_FS */
	return 0;
}


#include <linux/netdevice.h>

int __init dev_proc_init(void)
{
	lx_emul_trace(__func__);
	return 0;
}


extern void unregister_handler_proc(unsigned int irq,struct irqaction * action);
void unregister_handler_proc(unsigned int irq,struct irqaction * action)
{
	lx_emul_trace(__func__);
}


#include <asm-generic/qspinlock.h>

void queued_spin_lock_slowpath(struct qspinlock * lock,u32 val)
{
	lx_emul_trace(__func__);
}


#include <linux/filter.h>

DEFINE_PER_CPU(struct bpf_redirect_info, bpf_redirect_info);


#include <linux/rhashtable-types.h>

int rhashtable_init(struct rhashtable * ht,const struct rhashtable_params * params)
{
	lx_emul_trace(__func__);
	return -EINVAL;
}


#include <linux/wait_bit.h>

void __init wait_bit_init(void)
{
	lx_emul_trace(__func__);
}


#include <linux/regulator/consumer.h>

int regulator_enable(struct regulator * regulator)
{
	lx_emul_trace(__func__);
	return 0;
}
