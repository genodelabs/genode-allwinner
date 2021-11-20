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

unsigned long __must_check __arch_copy_from_user(void *to, const void __user *from, unsigned long n);
unsigned long __must_check __arch_copy_from_user(void *to, const void __user *from, unsigned long n)
{
	lx_emul_trace_and_stop(__func__);
}


unsigned long __must_check __arch_copy_to_user(void __user *to, const void *from, unsigned long n);
unsigned long __must_check __arch_copy_to_user(void __user *to, const void *from, unsigned long n)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/of_reserved_mem.h>
#include <linux/mod_devicetable.h>

const struct of_device_id __reservedmem_of_table[] = {};


#include <asm-generic/sections.h>

char __start_rodata[] = {};
char __end_rodata[]   = {};


#include <linux/tracepoint-defs.h>

const struct trace_print_flags gfpflag_names[]  = {};
const struct trace_print_flags pageflag_names[] = {};
const struct trace_print_flags vmaflag_names[]  = {};


#include <linux/kernel_stat.h>
#include <linux/stat.h>

struct kernel_stat kstat;


#include <linux/prandom.h>

DEFINE_PER_CPU(unsigned long, net_rand_noise);
EXPORT_PER_CPU_SYMBOL(net_rand_noise);


#include <linux/rcupdate.h>

void rcu_read_unlock_strict(void)
{
	lx_emul_trace(__func__);
}


#include <asm/memory.h>

u64 vabits_actual;


#include <linux/cpuhotplug.h>

int __cpuhp_setup_state(enum cpuhp_state state,const char * name,bool invoke,
                        int (* startup)(unsigned int cpu),
                        int (* teardown)(unsigned int cpu), bool multi_instance)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/timekeeper_internal.h>

void update_vsyscall(struct timekeeper * tk)
{
	lx_emul_trace(__func__);
}


#include <linux/sched/signal.h>

void ignore_signals(struct task_struct * t)
{
	lx_emul_trace(__func__);
}


#include <linux/sched.h>

void do_set_cpus_allowed(struct task_struct * p,const struct cpumask * new_mask)
{
	lx_emul_trace(__func__);
}


#include <linux/sysfs.h>

int sysfs_create_dir_ns(struct kobject * kobj,const void * ns)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/sysfs.h>

void sysfs_remove_dir(struct kobject * kobj)
{
	lx_emul_trace(__func__);
}


#include <linux/sysfs.h>

int sysfs_create_groups(struct kobject * kobj,const struct attribute_group ** groups)
{
	return 0;
}


#include <linux/sysfs.h>

void sysfs_remove_groups(struct kobject * kobj,const struct attribute_group ** groups)
{
	lx_emul_trace(__func__);
}


#include <linux/sysfs.h>

int sysfs_create_bin_file(struct kobject * kobj,const struct bin_attribute * attr)
{
	return 0;
}


#include <linux/sysfs.h>

int sysfs_create_link(struct kobject * kobj,struct kobject * target,const char * name)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/sysfs.h>

int sysfs_create_file_ns(struct kobject * kobj,const struct attribute * attr,const void * ns)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/sysfs.h>

void sysfs_remove_link(struct kobject * kobj,const char * name) { }


#include <linux/sysfs.h>

void sysfs_delete_link(struct kobject * kobj,struct kobject * targ,const char * name)
{
	lx_emul_trace(__func__);
}


#include <linux/kernfs.h>

void kernfs_get(struct kernfs_node * kn) { }


#include <linux/kernfs.h>

void kernfs_put(struct kernfs_node * kn) { }


#include <linux/kernfs.h>

struct kernfs_node * kernfs_find_and_get_ns(struct kernfs_node * parent,const char * name,const void * ns)
{
	return NULL;
}


#include <linux/kobject.h>

int kobject_uevent(struct kobject * kobj,enum kobject_action action)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/proc_fs.h>

struct proc_dir_entry * proc_symlink(const char * name,struct proc_dir_entry * parent,const char * dest)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/property.h>

int software_node_notify(struct device * dev,unsigned long action)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/random.h>

int __must_check get_random_bytes_arch(void * buf,int nbytes)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/random.h>

int add_random_ready_callback(struct random_ready_callback * rdy)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/logic_pio.h>

struct logic_pio_hwaddr * find_io_range_by_fwnode(struct fwnode_handle * fwnode)
{
	lx_emul_trace(__func__);
	return NULL;
}


#include <linux/syscore_ops.h>

void register_syscore_ops(struct syscore_ops * ops)
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


#include <linux/sched/loadavg.h>

void calc_global_load(void)
{
	lx_emul_trace(__func__);
}


#include <linux/kernel_stat.h>

void account_process_tick(struct task_struct * p,int user_tick)
{
	lx_emul_trace(__func__);
}


#include <linux/rcupdate.h>

void rcu_sched_clock_irq(int user)
{
	lx_emul_trace(__func__);
}


bool arm64_use_ng_mappings = false;
EXPORT_SYMBOL(arm64_use_ng_mappings);


#include <asm/pgtable.h>

pgprot_t pgprot_writecombine(pgprot_t prot)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kernel.h>

bool parse_option_str(const char *str, const char *option)
{
	lx_emul_trace(__func__);
	return false;
}


#include <linux/sysfs.h>

void sysfs_remove_file_ns(struct kobject * kobj,const struct attribute * attr,const void * ns)
{
	lx_emul_trace(__func__);
}


#include <linux/dma-mapping.h>

void arch_setup_dma_ops(struct device * dev,u64 dma_base,u64 size,const struct iommu_ops * iommu,bool coherent)
{
	lx_emul_trace(__func__);
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


#include <linux/rcupdate.h>

void call_rcu(struct rcu_head * head,rcu_callback_t func)
{
	lx_emul_trace(__func__);
}


extern void register_irq_proc(unsigned int irq,struct irq_desc * desc);
void register_irq_proc(unsigned int irq,struct irq_desc * desc)
{
	lx_emul_trace(__func__);
}


extern void register_handler_proc(unsigned int irq,struct irqaction * action);
void register_handler_proc(unsigned int irq,struct irqaction * action)
{
	lx_emul_trace(__func__);
}


#include <linux/sysfs.h>

int sysfs_create_link_nowarn(struct kobject * kobj,struct kobject * target,const char * name)
{
	lx_emul_trace(__func__);
	return 0;
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


#include <asm-generic/delay.h>

void __udelay(unsigned long usecs)
{
	lx_emul_trace(__func__);
}


void __const_udelay(unsigned long xloops)
{
	lx_emul_trace(__func__);
}


#include <linux/srcutree.h>

void synchronize_srcu(struct srcu_struct * ssp)
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


#include <linux/sched/wake_q.h>

void wake_up_q(struct wake_q_head * head)
{
	lx_emul_trace(__func__);
}
