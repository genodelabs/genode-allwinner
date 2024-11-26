/*
 * \brief  Dummy definitions of Linux Kernel functions
 * \author Automatically generated file - do no edit
 * \date   2024-11-28
 */

#include <lx_emul.h>


#include <linux/ratelimit_types.h>

int ___ratelimit(struct ratelimit_state * rs,const char * func)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

struct clk_hw * __clk_hw_register_gate(struct device * dev,struct device_node * np,const char * name,const char * parent_name,const struct clk_hw * parent_hw,const struct clk_parent_data * parent_data,unsigned long flags,void __iomem * reg,u8 bit_idx,u8 clk_gate_flags,spinlock_t * lock)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/cpumask.h>

struct cpumask __cpu_active_mask;


#include <linux/dma-fence-unwrap.h>

struct dma_fence * __dma_fence_unwrap_merge(unsigned int num_fences,struct dma_fence ** fences,struct dma_fence_unwrap * iter)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/serial_core.h>

const struct earlycon_id __earlycon_table[] = {};


#include <linux/serial_core.h>

const struct earlycon_id __earlycon_table_end[] = {};


#include <linux/file.h>

unsigned long __fdget(unsigned int fd)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kfifo.h>

int __kfifo_alloc(struct __kfifo * fifo,unsigned int size,size_t esize,gfp_t gfp_mask)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kfifo.h>

void __kfifo_free(struct __kfifo * fifo)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kfifo.h>

unsigned int __kfifo_in(struct __kfifo * fifo,const void * buf,unsigned int len)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kfifo.h>

unsigned int __kfifo_out(struct __kfifo * fifo,void * buf,unsigned int len)
{
	lx_emul_trace_and_stop(__func__);
}


#include <asm-generic/percpu.h>

unsigned long __per_cpu_offset[NR_CPUS] = {};


#include <linux/printk.h>

void __printk_safe_enter(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

void __printk_safe_exit(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/task.h>

void __put_task_struct(struct task_struct * tsk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/mm.h>

void __show_mem(unsigned int filter,nodemask_t * nodemask,int max_zone_idx)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

pid_t __task_pid_nr_ns(struct task_struct * task,enum pid_type type,struct pid_namespace * ns)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

void __unregister_chrdev(unsigned int major,unsigned int baseminor,unsigned int count,const char * name)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/vmalloc.h>

void * __vmalloc_node_range(unsigned long size,unsigned long align,unsigned long start,unsigned long end,gfp_t gfp_mask,pgprot_t prot,unsigned long vm_flags,int node,const void * caller)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

int _printk_deferred(const char * fmt,...)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/mm.h>

atomic_long_t _totalram_pages;


#include <linux/random.h>

void __init add_bootloader_randomness(const void * buf,size_t len)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kobject.h>

int add_uevent_var(struct kobj_uevent_env * env,const char * format,...)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/amba/bus.h>

int amba_device_add(struct amba_device * dev,struct resource * parent)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/amba/bus.h>

struct amba_device * amba_device_alloc(const char * name,resource_size_t base,size_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/amba/bus.h>

void amba_device_put(struct amba_device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/anon_inodes.h>

struct file * anon_inode_getfile(const char * name,const struct file_operations * fops,void * priv,int flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/async.h>

async_cookie_t async_schedule_node(async_func_t func,void * data,int node)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/async.h>

void async_synchronize_full(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/blkdev.h>

struct class block_class;


#include <linux/init.h>

char __initdata boot_command_line[] = {};


#include <linux/bsearch.h>

void * bsearch(const void * key,const void * base,size_t num,size_t size,cmp_func_t cmp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kernel.h>

void bust_spinlocks(int yes)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

unsigned long clk_hw_get_flags(const struct clk_hw * hw)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

unsigned int clk_hw_get_num_parents(const struct clk_hw * hw)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

struct clk_hw * clk_hw_get_parent(const struct clk_hw * hw)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

struct clk_hw * clk_hw_get_parent_by_index(const struct clk_hw * hw,unsigned int index)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

unsigned long clk_hw_get_rate(const struct clk_hw * hw)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

unsigned long clk_hw_round_rate(struct clk_hw * hw,unsigned long rate)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

void clk_hw_unregister(struct clk_hw * hw)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

void clk_hw_unregister_gate(struct clk_hw * hw)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk.h>

void clk_rate_exclusive_put(struct clk * clk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk.h>

long clk_round_rate(struct clk * clk,unsigned long rate)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/console.h>

void console_flush_on_panic(enum con_flush_mode mode)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/console.h>

void console_lock(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/console.h>

void console_unblank(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/console.h>

void console_unlock(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

void console_verbose(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/cpumask.h>

unsigned int cpumask_any_distribute(const struct cpumask * srcp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/topology.h>

bool cpus_share_cache(int this_cpu,int that_cpu)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

struct file * dentry_open(const struct path * path,int flags,const struct cred * cred)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/property.h>

int device_add_software_node(struct device * dev,const struct software_node * node)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/property.h>

int device_create_managed_software_node(struct device * dev,const struct property_entry * properties,const struct software_node * parent)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/property.h>

void device_remove_software_node(struct device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

struct clk * devm_clk_register(struct device * dev,struct clk_hw * hw)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reset-controller.h>

int devm_reset_controller_register(struct device * dev,struct reset_controller_dev * rcdev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

int divider_get_val(unsigned long rate,unsigned long parent_rate,const struct clk_div_table * table,u8 width,unsigned long flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

unsigned long divider_recalc_rate(struct clk_hw * hw,unsigned long parent_rate,unsigned int val,const struct clk_div_table * table,unsigned long flags,unsigned long width)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

long divider_round_rate_parent(struct clk_hw * hw,struct clk_hw * parent,unsigned long rate,unsigned long * prate,const struct clk_div_table * table,u8 width,unsigned long flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-mapping.h>

struct page * dma_alloc_pages(struct device * dev,size_t size,dma_addr_t * dma_handle,enum dma_data_direction dir,gfp_t gfp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

struct dma_buf_attachment * dma_buf_attach(struct dma_buf * dmabuf,struct device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

void dma_buf_detach(struct dma_buf * dmabuf,struct dma_buf_attachment * attach)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

struct dma_buf * dma_buf_export(const struct dma_buf_export_info * exp_info)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

int dma_buf_fd(struct dma_buf * dmabuf,int flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

struct dma_buf * dma_buf_get(int fd)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

struct sg_table * dma_buf_map_attachment_unlocked(struct dma_buf_attachment * attach,enum dma_data_direction direction)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

void dma_buf_put(struct dma_buf * dmabuf)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

void dma_buf_unmap_attachment_unlocked(struct dma_buf_attachment * attach,struct sg_table * sg_table,enum dma_data_direction direction)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

void dma_buf_vunmap_unlocked(struct dma_buf * dmabuf,struct iosys_map * map)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-map-ops.h>

bool dma_default_coherent;


#include <linux/dma-mapping.h>

void dma_free_pages(struct device * dev,size_t size,struct page * page,dma_addr_t dma_handle,enum dma_data_direction dir)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-mapping.h>

int dma_get_sgtable_attrs(struct device * dev,struct sg_table * sgt,void * cpu_addr,dma_addr_t dma_addr,size_t size,unsigned long attrs)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-mapping.h>

int dma_map_sgtable(struct device * dev,struct sg_table * sgt,enum dma_data_direction dir,unsigned long attrs)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-mapping.h>

int dma_mmap_attrs(struct device * dev,struct vm_area_struct * vma,void * cpu_addr,dma_addr_t dma_addr,size_t size,unsigned long attrs)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-mapping.h>

int dma_mmap_pages(struct device * dev,struct vm_area_struct * vma,size_t size,struct page * page)
{
	lx_emul_trace_and_stop(__func__);
}


#include <asm-generic/softirq_stack.h>

void do_softirq_own_stack(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <drm/display/drm_hdmi_helper.h>

int drm_hdmi_infoframe_set_hdr_metadata(struct hdmi_drm_infoframe * frame,const struct drm_connector_state * conn_state)
{
	lx_emul_trace_and_stop(__func__);
}


#include <drm/display/drm_scdc_helper.h>

ssize_t drm_scdc_read(struct i2c_adapter * adapter,u8 offset,void * buffer,size_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <drm/display/drm_scdc_helper.h>

bool drm_scdc_set_high_tmds_clock_ratio(struct drm_connector * connector,bool set)
{
	lx_emul_trace_and_stop(__func__);
}


#include <drm/display/drm_scdc_helper.h>

bool drm_scdc_set_scrambling(struct drm_connector * connector,bool enable)
{
	lx_emul_trace_and_stop(__func__);
}


#include <drm/display/drm_scdc_helper.h>

ssize_t drm_scdc_write(struct i2c_adapter * adapter,u8 offset,const void * buffer,size_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

asmlinkage __visible void dump_stack(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

asmlinkage __visible void dump_stack_lvl(const char * log_lvl)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reboot.h>

void emergency_restart(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/extcon.h>

struct extcon_dev * extcon_get_edev_by_phandle(struct device * dev,int index)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/extcon.h>

int extcon_get_state(struct extcon_dev * edev,const unsigned int id)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/extcon.h>

int extcon_register_notifier_all(struct extcon_dev * edev,struct notifier_block * nb)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/extcon.h>

int extcon_unregister_notifier_all(struct extcon_dev * edev,struct notifier_block * nb)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fb.h>

ssize_t fb_sys_read(struct fb_info * info,char __user * buf,size_t count,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fb.h>

ssize_t fb_sys_write(struct fb_info * info,const char __user * buf,size_t count,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/file.h>

void fd_install(unsigned int fd,struct file * file)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/rcuwait.h>

void finish_rcuwait(struct rcuwait * w)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kobject.h>

struct kobject *firmware_kobj;


#include <linux/mmzone.h>

struct pglist_data * first_online_pgdat(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/file.h>

void fput(struct file * file)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kernel.h>

int get_option(char ** str,int * pint)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/file.h>

int get_unused_fd_flags(unsigned flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gfp.h>

bool gfp_pfmemalloc_allowed(gfp_t gfp_mask)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uuid.h>

const u8 guid_index[16] = {};


#include <linux/hdmi.h>

void hdmi_avi_infoframe_init(struct hdmi_avi_infoframe * frame)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/hdmi.h>

ssize_t hdmi_drm_infoframe_pack(struct hdmi_drm_infoframe * frame,void * buffer,size_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/hdmi.h>

int hdmi_vendor_infoframe_init(struct hdmi_vendor_infoframe * frame)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/hdmi.h>

ssize_t hdmi_vendor_infoframe_pack(struct hdmi_vendor_infoframe * frame,void * buffer,size_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/i2c.h>

int i2c_add_adapter(struct i2c_adapter * adapter)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/i2c.h>

void i2c_del_adapter(struct i2c_adapter * adap)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/i2c.h>

struct i2c_adapter * i2c_get_adapter_by_fwnode(struct fwnode_handle * fwnode)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/i2c.h>

void i2c_put_adapter(struct i2c_adapter * adap)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/i2c.h>

int i2c_transfer(struct i2c_adapter * adap,struct i2c_msg * msgs,int num)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pseudo_fs.h>

struct pseudo_fs_context * init_pseudo(struct fs_context * fc,unsigned long magic)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/init.h>

bool initcall_debug;


#include <linux/ioport.h>

int insert_resource(struct resource * parent,struct resource * new)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

void __sched io_schedule(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

void io_schedule_finish(int token)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

int io_schedule_prepare(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/ioport.h>

struct resource iomem_resource;


#include <linux/ioport.h>

struct resource ioport_resource;


#include <asm-generic/io.h>

void __iomem * ioremap_prot(phys_addr_t phys_addr,size_t size,unsigned long prot)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

void iput(struct inode * inode)
{
	lx_emul_trace_and_stop(__func__);
}


extern bool irq_wait_for_poll(struct irq_desc * desc);
bool irq_wait_for_poll(struct irq_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/irq_work.h>

bool irq_work_queue(struct irq_work * work)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/irq_work.h>

void irq_work_tick(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/property.h>

bool is_software_node(const struct fwnode_handle * fwnode)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kobject.h>

struct kobject *kernel_kobj;


#include <linux/sched.h>

void kick_process(struct task_struct * p)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

void kill_anon_super(struct super_block * sb)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kmsg_dump.h>

void kmsg_dump(enum kmsg_dump_reason reason)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kobject.h>

int kobject_synth_uevent(struct kobject * kobj,const char * buf,size_t count)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/llist.h>

bool llist_add_batch(struct llist_node * new_first,struct llist_node * new_last,struct llist_head * head)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/logic_pio.h>

unsigned long logic_pio_trans_hwaddr(struct fwnode_handle * fwnode,resource_size_t addr,resource_size_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/delay.h>

unsigned long lpj_fine;


#include <linux/string.h>

ssize_t memory_read_from_buffer(void * to,size_t count,loff_t * ppos,const void * from,size_t available)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/mmzone.h>

struct pglist_data * next_online_pgdat(struct pglist_data * pgdat)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/irq.h>

struct irq_chip no_irq_chip;


#include <linux/fs.h>

int nonseekable_open(struct inode * inode,struct file * filp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

loff_t noop_llseek(struct file * file,loff_t offset,int whence)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/irq.h>

void note_interrupt(struct irq_desc * desc,irqreturn_t action_ret)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

void of_clk_del_provider(struct device_node * np)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/of_clk.h>

const char * of_clk_get_parent_name(const struct device_node * np,int index)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk-provider.h>

struct clk_hw * of_clk_hw_onecell_get(struct of_phandle_args * clkspec,void * data)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/of.h>

ssize_t of_modalias(const struct device_node * np,char * str,ssize_t len)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/serial_core.h>

int __init of_setup_earlycon(const struct earlycon_id * match,unsigned long node,const char * options)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

int oops_in_progress;	/* If set, an oops, panic(), BUG() or die() is in progress */


#include <linux/highuid.h>

int overflowuid;


#include <linux/reboot.h>

enum reboot_mode panic_reboot_mode;


#include <linux/initrd.h>

unsigned long phys_initrd_size;


#include <linux/initrd.h>

phys_addr_t phys_initrd_start;


#include <linux/sysctl.h>

int proc_dointvec_minmax(struct ctl_table * table,int write,void * buffer,size_t * lenp,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sysctl.h>

int proc_douintvec(struct ctl_table * table,int write,void * buffer,size_t * lenp,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pid.h>

void put_pid(struct pid * pid)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/file.h>

void put_unused_fd(unsigned int fd)
{
	lx_emul_trace_and_stop(__func__);
}


extern void raw_spin_rq_lock_nested(struct rq * rq,int subclass);
void raw_spin_rq_lock_nested(struct rq * rq,int subclass)
{
	lx_emul_trace_and_stop(__func__);
}


extern void raw_spin_rq_unlock(struct rq * rq);
void raw_spin_rq_unlock(struct rq * rq)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reboot.h>

enum reboot_mode reboot_mode;


#include <linux/regulator/consumer.h>

void regulator_bulk_unregister_supply_alias(struct device * dev,const char * const * id,int num_id)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/regulator/consumer.h>

int regulator_disable(struct regulator * regulator)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/regulator/consumer.h>

void regulator_put(struct regulator * regulator)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/ioport.h>

int release_resource(struct resource * old)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/mm.h>

void __meminit reserve_bootmem_region(phys_addr_t start,phys_addr_t end,int nid)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reset.h>

int reset_control_assert(struct reset_control * rstc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reset-controller.h>

void reset_controller_unregister(struct reset_controller_dev * rcdev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

void seq_printf(struct seq_file * m,const char * f,...)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

void seq_vprintf(struct seq_file * m,const char * f,va_list args)
{
	lx_emul_trace_and_stop(__func__);
}


extern void set_rq_offline(struct rq * rq);
void set_rq_offline(struct rq * rq)
{
	lx_emul_trace_and_stop(__func__);
}


extern void set_rq_online(struct rq * rq);
void set_rq_online(struct rq * rq)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/scatterlist.h>

void sg_free_table(struct sg_table * table)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/scatterlist.h>

struct scatterlist * sg_next(struct scatterlist * sg)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/debug.h>

void show_state_filter(unsigned int state_filter)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/siphash.h>

u64 siphash_1u64(const u64 first,const siphash_key_t * key)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/smp.h>

void smp_call_function_many(const struct cpumask * mask,smp_call_func_t func,void * info,bool wait)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/smp.h>

int smp_call_function_single(int cpu,smp_call_func_t func,void * info,int wait)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/smp.h>

void smp_send_stop(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/jump_label.h>

bool static_key_initialized;


#include <linux/printk.h>

int suppress_printk;


extern struct gpio_desc * swnode_find_gpio(struct fwnode_handle * fwnode,const char * con_id,unsigned int idx,unsigned long * flags);
struct gpio_desc * swnode_find_gpio(struct fwnode_handle * fwnode,const char * con_id,unsigned int idx,unsigned long * flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sync_file.h>

struct sync_file * sync_file_create(struct dma_fence * fence)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sync_file.h>

struct dma_fence * sync_file_get_fence(int fd)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/rcupdate.h>

void synchronize_rcu(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sysctl.h>

const int sysctl_vals[] = {};


#include <linux/sysfs.h>

void sysfs_notify(struct kobject * kobj,const char * dir,const char * attr)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clockchips.h>

void tick_broadcast(const struct cpumask * mask)
{
	lx_emul_trace_and_stop(__func__);
}


extern void unregister_handler_proc(unsigned int irq,struct irqaction * action);
void unregister_handler_proc(unsigned int irq,struct irqaction * action)
{
	lx_emul_trace_and_stop(__func__);
}


extern void unregister_irq_proc(unsigned int irq,struct irq_desc * desc);
void unregister_irq_proc(unsigned int irq,struct irq_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


extern void update_group_capacity(struct sched_domain * sd,int cpu);
void update_group_capacity(struct sched_domain * sd,int cpu)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uuid.h>

const u8 uuid_index[16] = {};


#include <linux/vmalloc.h>

void vfree(const void * addr)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/mm.h>

pgprot_t vm_get_page_prot(unsigned long vm_flags)
{
	lx_emul_trace_and_stop(__func__);
}

