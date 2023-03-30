/*
 * \brief  Dummy definitions of Linux Kernel functions
 * \author Automatically generated file - do no edit
 * \date   2021-11-24
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


extern void __memcpy_fromio(void * to,const volatile void __iomem * from,size_t count);
void __memcpy_fromio(void * to,const volatile void __iomem * from,size_t count)
{
	lx_emul_trace_and_stop(__func__);
}


extern void __memcpy_toio(volatile void __iomem * to,const void * from,size_t count);
void __memcpy_toio(volatile void __iomem * to,const void * from,size_t count)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reset.h>

struct reset_control * __of_reset_control_get(struct device_node * node,const char * id,int index,bool shared,bool optional,bool acquired)
{
	lx_emul_trace_and_stop(__func__);
}


#include <asm-generic/percpu.h>

unsigned long __per_cpu_offset[NR_CPUS] = {};


#include <linux/sched/task.h>

void __put_task_struct(struct task_struct * tsk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/ioport.h>

void __release_region(struct resource * parent,resource_size_t start,resource_size_t n)
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

void * __vmalloc_node(unsigned long size,unsigned long align,gfp_t gfp_mask,int node,const void * caller)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/random.h>

void add_bootloader_randomness(const void * buf,unsigned int size)
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

const struct clk_ops clk_fixed_factor_ops;


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


#include <linux/printk.h>

int console_printk[] = {};


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

struct sg_table * dma_buf_map_attachment(struct dma_buf_attachment * attach,enum dma_data_direction direction)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

void dma_buf_put(struct dma_buf * dmabuf)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

void dma_buf_unmap_attachment(struct dma_buf_attachment * attach,struct sg_table * sg_table,enum dma_data_direction direction)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

int dma_buf_vmap(struct dma_buf * dmabuf,struct dma_buf_map * map)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-buf.h>

void dma_buf_vunmap(struct dma_buf * dmabuf,struct dma_buf_map * map)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-map-ops.h>

bool dma_default_coherent;


#include <linux/dma-fence.h>

int dma_fence_add_callback(struct dma_fence * fence,struct dma_fence_cb * cb,dma_fence_func_t func)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

struct dma_fence * dma_fence_allocate_private_stub(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence-chain.h>

int dma_fence_chain_find_seqno(struct dma_fence ** pfence,uint64_t seqno)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence-chain.h>

void dma_fence_chain_init(struct dma_fence_chain * chain,struct dma_fence * prev,struct dma_fence * fence,uint64_t seqno)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence-chain.h>

const struct dma_fence_ops dma_fence_chain_ops;


#include <linux/dma-fence-chain.h>

struct dma_fence * dma_fence_chain_walk(struct dma_fence * fence)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

struct dma_fence * dma_fence_get_stub(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

void dma_fence_init(struct dma_fence * fence,const struct dma_fence_ops * ops,spinlock_t * lock,u64 context,u64 seqno)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

void dma_fence_release(struct kref * kref)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

bool dma_fence_remove_callback(struct dma_fence * fence,struct dma_fence_cb * cb)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

int dma_fence_signal(struct dma_fence * fence)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

int dma_fence_signal_timestamp(struct dma_fence * fence,ktime_t timestamp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-fence.h>

signed long dma_fence_wait_timeout(struct dma_fence * fence,bool intr,signed long timeout)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dma-mapping.h>

int dma_get_sgtable_attrs(struct device * dev,struct sg_table * sgt,void * cpu_addr,dma_addr_t dma_addr,size_t size,unsigned long attrs)
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


#include <linux/dma-resv.h>

void dma_resv_fini(struct dma_resv * obj)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

asmlinkage __visible void dump_stack(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reboot.h>

void emergency_restart(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/file.h>

void fd_install(unsigned int fd,struct file * file)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kobject.h>

struct kobject *firmware_kobj;


#include <linux/file.h>

void fput(struct file * file)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/property.h>

struct fwnode_handle * fwnode_create_software_node(const struct property_entry * properties,const struct fwnode_handle * parent)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/property.h>

void fwnode_remove_software_node(struct fwnode_handle * fwnode)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/file.h>

int get_unused_fd_flags(unsigned flags)
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

ssize_t hdmi_avi_infoframe_pack(struct hdmi_avi_infoframe * frame,void * buffer,size_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/hdmi.h>

int hdmi_drm_infoframe_init(struct hdmi_drm_infoframe * frame)
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

void irq_work_tick(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/property.h>

bool is_software_node(const struct fwnode_handle * fwnode)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/mm.h>

bool is_vmalloc_addr(const void * x)
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


#include <linux/memblock.h>

int __init_memblock memblock_add(phys_addr_t base,phys_addr_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

int __init_memblock memblock_clear_nomap(phys_addr_t base,phys_addr_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

phys_addr_t __init_memblock memblock_find_in_range(phys_addr_t start,phys_addr_t end,phys_addr_t size,phys_addr_t align)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

int __init_memblock memblock_free(phys_addr_t base,phys_addr_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

bool __init_memblock memblock_is_region_reserved(phys_addr_t base,phys_addr_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

int __init_memblock memblock_mark_hotplug(phys_addr_t base,phys_addr_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

int __init_memblock memblock_mark_nomap(phys_addr_t base,phys_addr_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/memblock.h>

int __init_memblock memblock_reserve(phys_addr_t base,phys_addr_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/string.h>

ssize_t memory_read_from_buffer(void * to,size_t count,loff_t * ppos,const void * from,size_t available)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/irq.h>

struct irq_chip no_irq_chip;


#include <linux/fs.h>

loff_t no_llseek(struct file * file,loff_t offset,int whence)
{
	lx_emul_trace_and_stop(__func__);
}


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


#include <linux/clk.h>

struct clk * of_clk_get_by_name(struct device_node * np,const char * name)
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


#include <linux/i2c.h>

struct i2c_adapter * of_get_i2c_adapter_by_node(struct device_node * node)
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


#include <linux/printk.h>

int printk_deferred(const char * fmt,...)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

void printk_safe_flush_on_panic(void)
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


#include <asm-generic/qrwlock.h>

void queued_read_lock_slowpath(struct qrwlock * lock)
{
	lx_emul_trace_and_stop(__func__);
}


#include <asm-generic/qspinlock.h>

void queued_spin_lock_slowpath(struct qspinlock * lock,u32 val)
{
	lx_emul_trace_and_stop(__func__);
}


#include <asm-generic/qrwlock.h>

void queued_write_lock_slowpath(struct qrwlock * lock)
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


#include <linux/reset.h>

int reset_control_assert(struct reset_control * rstc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reset.h>

void reset_control_put(struct reset_control * rstc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

void seq_vprintf(struct seq_file * m,const char * f,va_list args)
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


#include <linux/mm.h>

void show_mem(unsigned int filter,nodemask_t * nodemask)
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


#include <linux/string_helpers.h>

int string_escape_mem(const char * src,size_t isz,char * dst,size_t osz,unsigned int flags,const char * only)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

int suppress_printk;


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


#include <linux/vt_kern.h>

void unblank_screen(void)
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


#include <linux/vmalloc.h>

void * vzalloc(unsigned long size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/wake_q.h>

void wake_q_add_safe(struct wake_q_head * head,struct task_struct * task)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/wait_bit.h>

void wake_up_var(void * var)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/rcutree.h>

void rcu_irq_enter_irqson(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/rcutree.h>

void rcu_irq_exit_irqson(void)
{
	lx_emul_trace_and_stop(__func__);
}
