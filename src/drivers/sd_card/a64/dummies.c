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

#include <../block/blk.h>
struct kobj_type blk_queue_ktype;


#include <linux/prandom.h>

DEFINE_PER_CPU(unsigned long, net_rand_noise);
EXPORT_PER_CPU_SYMBOL(net_rand_noise);


#include <linux/kernfs.h>

struct kernfs_node * kernfs_find_and_get_ns(struct kernfs_node * parent,const char * name,const void * ns)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/random.h>

void get_random_bytes(void * buf,int nbytes)
{
	lx_emul_trace(__func__);
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


#include <linux/pinctrl/devinfo.h>

int pinctrl_bind_pins(struct device * dev)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/pinctrl/devinfo.h>

int pinctrl_init_done(struct device * dev)
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


#include <linux/regulator/consumer.h>

int regulator_disable(struct regulator * regulator)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/regulator/consumer.h>

int regulator_count_voltages(struct regulator * regulator)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/regulator/consumer.h>

int regulator_set_voltage(struct regulator * regulator,int min_uV,int max_uV)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/regulator/consumer.h>

int regulator_enable(struct regulator * regulator)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/regulator/consumer.h>

int regulator_is_supported_voltage(struct regulator * regulator,int min_uV,int max_uV)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/reset.h>

struct reset_control * __devm_reset_control_get(struct device * dev,const char * id,int index,bool shared,bool optional,bool acquired)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/reset.h>

int reset_control_reset(struct reset_control * rstc)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/gpio/consumer.h>

struct gpio_desc * __must_check devm_gpiod_get_index(struct device * dev,const char * con_id,unsigned int idx,enum gpiod_flags flags)
{
	// XXX GPIO for card-detection
	lx_emul_trace(__func__);
	return ERR_PTR(-ENOSYS);
}


#include <linux/gpio/consumer.h>

int gpiod_set_debounce(struct gpio_desc * desc,unsigned int debounce)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/task_work.h>

int task_work_add(struct task_struct * task,struct callback_head * work,enum task_work_notify_mode notify)
{
	lx_emul_trace(__func__);
	// XXX task_work_add AFAICT only called from irq_thread (kernel/irq/manage.c)
	return -1;
}


#include <linux/random.h>

void add_interrupt_randomness(int irq,int irq_flags)
{
	lx_emul_trace(__func__);
}


#include <linux/prandom.h>

u32 prandom_u32(void)
{
	lx_emul_trace(__func__);
	return (u32) 0xdeadbeef;
}


#include <linux/backing-dev.h>

struct backing_dev_info * bdi_alloc(int node_id)
{
	struct backing_dev_info * ret =
		kzalloc(sizeof(struct backing_dev_info), GFP_KERNEL);
	return ret;
}


#include <linux/backing-dev.h>

void bdi_put(struct backing_dev_info * bdi)
{
	lx_emul_trace(__func__);
}


#include <linux/backing-dev.h>

int bdi_register(struct backing_dev_info * bdi,const char * fmt,...)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/backing-dev.h>

void bdi_set_owner(struct backing_dev_info * bdi,struct device * owner)
{
	lx_emul_trace(__func__);
}


#include <linux/backing-dev.h>

void bdi_unregister(struct backing_dev_info * bdi)
{
	lx_emul_trace(__func__);
}


#include <linux/blkdev.h>

void bdput(struct block_device * bdev)
{
	lx_emul_trace(__func__);
}


#include <../block/blk-stat.h>
struct blk_queue_stats { int dummy; };

struct blk_queue_stats * blk_alloc_queue_stats(void)
{
    static struct blk_queue_stats ret;
    lx_emul_trace(__func__);
    return &ret;
}


extern struct blk_stat_callback * blk_stat_alloc_callback(void (* timer_fn)(struct blk_stat_callback *),int (* bucket_fn)(const struct request *),unsigned int buckets,void * data);
struct blk_stat_callback * blk_stat_alloc_callback(void (* timer_fn)(struct blk_stat_callback *),int (* bucket_fn)(const struct request *),unsigned int buckets,void * data)
{
	static struct blk_stat_callback ret;
	lx_emul_trace(__func__);
	return &ret;
}


extern void blk_mq_sysfs_init(struct request_queue * q);
void blk_mq_sysfs_init(struct request_queue * q)
{
	lx_emul_trace(__func__);
}


extern void blk_mq_hctx_kobj_init(struct blk_mq_hw_ctx * hctx);
void blk_mq_hctx_kobj_init(struct blk_mq_hw_ctx * hctx)
{
	lx_emul_trace(__func__);
}


#include <linux/blkdev.h>

void blkdev_put(struct block_device * bdev,fmode_t mode)
{
	lx_emul_trace(__func__);
}


#include <linux/cpuhotplug.h>

int __cpuhp_state_add_instance(enum cpuhp_state state,struct hlist_node * node,bool invoke)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/genhd.h>

void rand_initialize_disk(struct gendisk * disk)
{
	lx_emul_trace(__func__);
}


#include <linux/wait_bit.h>

void __init wait_bit_init(void)
{
       lx_emul_trace(__func__);
}


#include <linux/mm.h>

bool is_vmalloc_addr(const void * x)
{
	lx_emul_trace(__func__);
	return false;
}
