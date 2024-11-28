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


#include <linux/cpumask.h>

struct cpumask __cpu_active_mask;


#include <crypto/utils.h>

noinline unsigned long __crypto_memneq(const void * a,const void * b,size_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <crypto/utils.h>

void __crypto_xor(u8 * dst,const u8 * src1,const u8 * src2,unsigned int len)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/serial_core.h>

const struct earlycon_id __earlycon_table[] = {};


#include <linux/serial_core.h>

const struct earlycon_id __earlycon_table_end[] = {};


#include <linux/ethtool.h>

int __ethtool_get_link_ksettings(struct net_device * dev,struct ethtool_link_ksettings * link_ksettings)
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


#include <linux/cred.h>

void __put_cred(struct cred * cred)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/task.h>

void __put_task_struct(struct task_struct * tsk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/scm.h>

void __scm_destroy(struct scm_cookie * scm)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/scm.h>

int __scm_send(struct socket * sock,struct msghdr * msg,struct scm_cookie * p)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/mm.h>

void __show_mem(unsigned int filter,nodemask_t * nodemask,int max_zone_idx)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/skbuff.h>

u32 __skb_get_hash_symmetric(const struct sk_buff * skb)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/gso.h>

struct sk_buff * __skb_gso_segment(struct sk_buff * skb,netdev_features_t features,bool tx_path)
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


#include <linux/blkdev.h>

struct class block_class;


#include <linux/init.h>

char __initdata boot_command_line[] = {};


#include <linux/filter.h>

int bpf_prog_create_from_user(struct bpf_prog ** pfp,struct sock_fprog * fprog,bpf_aux_classic_check_t trans,bool save_orig)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/filter.h>

void bpf_prog_destroy(struct bpf_prog * fp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/filter.h>

void bpf_warn_invalid_xdp_action(struct net_device * dev,struct bpf_prog * prog,u32 act)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/kernel.h>

void bust_spinlocks(int yes)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk.h>

int clk_set_phase(struct clk * clk,int degrees)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/console.h>

void console_flush_on_panic(enum con_flush_mode mode)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/console.h>

void console_unblank(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

void console_verbose(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/filter.h>

int copy_bpf_fprog_from_user(struct sock_fprog * dst,sockptr_t src,int len)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uio.h>

size_t copy_page_from_iter(struct page * page,size_t offset,size_t bytes,struct iov_iter * i)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

ssize_t copy_splice_read(struct file * in,loff_t * ppos,struct pipe_inode_info * pipe,size_t len,unsigned int flags)
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


#include <net/ip6_checksum.h>

__sum16 csum_ipv6_magic(const struct in6_addr * saddr,const struct in6_addr * daddr,__u32 len,__u8 proto,__wsum csum)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/netdevice.h>

int dev_ifconf(struct net * net,struct ifconf __user * uifc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/property.h>

int device_create_managed_software_node(struct device * dev,const struct property_entry * properties,const struct software_node * parent)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

struct gpio_desc * __must_check devm_gpiod_get(struct device * dev,const char * con_id,enum gpiod_flags flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/dst.h>

void dst_release(struct dst_entry * dst)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

asmlinkage __visible void dump_stack_lvl(const char * log_lvl)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/dcache.h>

char * dynamic_dname(char * buffer,int buflen,const char * fmt,...)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reboot.h>

void emergency_restart(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/ethtool.h>

u32 ethtool_op_get_link(struct net_device * dev)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/ethtool.h>

int ethtool_op_get_ts_info(struct net_device * dev,struct ethtool_ts_info * info)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

pid_t f_getown(struct file * filp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

int f_setown(struct file * filp,int who,int force)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

int fasync_helper(int fd,struct file * filp,int on,struct fasync_struct ** fapp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

struct task_struct * find_task_by_vpid(pid_t vnr)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/pid.h>

struct pid * find_vpid(int nr)
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


#include <net/flow_dissector.h>

struct flow_dissector flow_keys_basic_dissector;


#include <linux/sched/signal.h>

void flush_signals(struct task_struct * t)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/gen_stats.h>

void gen_kill_estimator(struct net_rate_estimator __rcu ** rate_est)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gfp.h>

bool gfp_pfmemalloc_allowed(gfp_t gfp_mask)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/gen_stats.h>

void gnet_stats_add_basic(struct gnet_stats_basic_sync * bstats,struct gnet_stats_basic_sync __percpu * cpu,struct gnet_stats_basic_sync * b,bool running)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/gen_stats.h>

void gnet_stats_add_queue(struct gnet_stats_queue * qstats,const struct gnet_stats_queue __percpu * cpu,const struct gnet_stats_queue * q)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/gen_stats.h>

int gnet_stats_copy_basic(struct gnet_dump * d,struct gnet_stats_basic_sync __percpu * cpu,struct gnet_stats_basic_sync * b,bool running)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/gen_stats.h>

int gnet_stats_copy_queue(struct gnet_dump * d,struct gnet_stats_queue __percpu * cpu_q,struct gnet_stats_queue * q,__u32 qlen)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

int gpiod_cansleep(const struct gpio_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

int gpiod_get_value(const struct gpio_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

int gpiod_get_value_cansleep(const struct gpio_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

int gpiod_is_active_low(const struct gpio_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

int gpiod_set_array_value_cansleep(unsigned int array_size,struct gpio_desc ** desc_array,struct gpio_array * array_info,unsigned long * value_bitmap)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

int gpiod_set_consumer_name(struct gpio_desc * desc,const char * name)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

int gpiod_set_debounce(struct gpio_desc * desc,unsigned int debounce)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

void gpiod_set_value(struct gpio_desc * desc,int value)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

void gpiod_set_value_cansleep(struct gpio_desc * desc,int value)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

int gpiod_to_irq(const struct gpio_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/gpio/consumer.h>

void gpiod_toggle_active_low(struct gpio_desc * desc)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uuid.h>

const u8 guid_index[16] = {};


#include <net/inet_common.h>

const struct proto_ops inet_dgram_ops;


#include <linux/utsname.h>

struct user_namespace init_user_ns;


#include <linux/init.h>

bool initcall_debug;


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


#include <linux/sched.h>

long __sched io_schedule_timeout(long timeout)
{
	lx_emul_trace_and_stop(__func__);
}


#include <asm-generic/io.h>

void __iomem * ioremap_prot(phys_addr_t phys_addr,size_t size,unsigned long prot)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uio.h>

void iov_iter_advance(struct iov_iter * i,size_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/uio.h>

void iov_iter_revert(struct iov_iter * i,size_t unroll)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/ip.h>

struct sk_buff * ip_check_defrag(struct net * net,struct sk_buff * skb,u32 user)
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


#include <linux/kobject.h>

struct kobject *kernel_kobj;


#include <linux/key.h>

key_ref_t key_create_or_update(key_ref_t keyring_ref,const char * type,const char * description,const void * payload,size_t plen,key_perm_t perm,unsigned long flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/key.h>

void key_put(struct key * key)
{
	lx_emul_trace_and_stop(__func__);
}


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


#include <linux/fs.h>

void kill_fasync(struct fasync_struct ** fp,int sig,int band)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/signal.h>

int kill_pid(struct pid * pid,int sig,int priv)
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


#include <linux/logic_pio.h>

unsigned long logic_pio_trans_hwaddr(struct fwnode_handle * fwnode,resource_size_t addr,resource_size_t size)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/delay.h>

unsigned long loops_per_jiffy;


#include <linux/delay.h>

unsigned long lpj_fine;


#include <linux/string.h>

ssize_t memory_read_from_buffer(void * to,size_t count,loff_t * ppos,const void * from,size_t available)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/preempt.h>

void migrate_disable(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/preempt.h>

void migrate_enable(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/neighbour.h>

const struct nla_policy nda_policy[] = {};


#include <linux/mmzone.h>

struct pglist_data * next_online_pgdat(struct pglist_data * pgdat)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/irq.h>

struct irq_chip no_irq_chip;


#include <linux/irq.h>

void note_interrupt(struct irq_desc * desc,irqreturn_t action_ret)
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


#include <linux/proc_fs.h>

int open_related_ns(struct ns_common * ns,struct ns_common * (* get_ns)(struct ns_common * ns))
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/page_pool/types.h>

void page_pool_destroy(struct page_pool * pool)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/page_pool/types.h>

void page_pool_put_defragged_page(struct page_pool * pool,struct page * page,unsigned int dma_sync_size,bool allow_direct)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reboot.h>

enum reboot_mode panic_reboot_mode;


#include <linux/moduleparam.h>

const struct kernel_param_ops param_ops_int;


#include <linux/moduleparam.h>

const struct kernel_param_ops param_ops_uint;


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


#include <linux/proc_ns.h>

void proc_free_inum(unsigned int inum)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/socket.h>

int put_cmsg(struct msghdr * msg,int level,int type,int len,void * data)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/socket.h>

void put_cmsg_scm_timestamping(struct msghdr * msg,struct scm_timestamping_internal * tss_internal)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/socket.h>

void put_cmsg_scm_timestamping64(struct msghdr * msg,struct scm_timestamping_internal * tss_internal)
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


#include <linux/rcuref.h>

bool rcuref_get_slowpath(rcuref_t * ref)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/reboot.h>

enum reboot_mode reboot_mode;


#include <linux/reboot.h>

int register_restart_handler(struct notifier_block * nb)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/regulator/consumer.h>

int regulator_list_voltage(struct regulator * regulator,unsigned selector)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/proc_fs.h>

void remove_proc_entry(const char * name,struct proc_dir_entry * parent)
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


#include <net/sock_reuseport.h>

int reuseport_detach_prog(struct sock * sk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/sock_reuseport.h>

void reuseport_detach_sock(struct sock * sk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/sock_reuseport.h>

void reuseport_update_incoming_cpu(struct sock * sk,int val)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/scm.h>

void scm_detach_fds(struct msghdr * msg,struct scm_cookie * scm)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

struct hlist_node * seq_hlist_next_rcu(void * v,struct hlist_head * head,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

struct hlist_node * seq_hlist_start_head_rcu(struct hlist_head * head,loff_t pos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

struct list_head * seq_list_next(void * v,struct list_head * head,loff_t * ppos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

struct list_head * seq_list_start(struct list_head * head,loff_t pos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

struct list_head * seq_list_start_head(struct list_head * head,loff_t pos)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

void seq_printf(struct seq_file * m,const char * f,...)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

void seq_putc(struct seq_file * m,char c)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/seq_file.h>

void seq_puts(struct seq_file * m,const char * s)
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


#include <crypto/sha2.h>

void sha224_final(struct sha256_state * sctx,u8 * out)
{
	lx_emul_trace_and_stop(__func__);
}


#include <crypto/sha2.h>

void sha256_final(struct sha256_state * sctx,u8 * out)
{
	lx_emul_trace_and_stop(__func__);
}


#include <crypto/sha2.h>

void sha256_update(struct sha256_state * sctx,const u8 * data,unsigned int len)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched/debug.h>

void show_state_filter(unsigned int state_filter)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

int simple_setattr(struct mnt_idmap * idmap,struct dentry * dentry,struct iattr * iattr)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

int simple_statfs(struct dentry * dentry,struct kstatfs * buf)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/filter.h>

int sk_attach_bpf(u32 ufd,struct sock * sk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/filter.h>

int sk_attach_filter(struct sock_fprog * fprog,struct sock * sk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/filter.h>

int sk_detach_filter(struct sock * sk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/filter.h>

void sk_filter_uncharge(struct sock * sk,struct sk_filter * fp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/filter.h>

int sk_reuseport_attach_bpf(u32 ufd,struct sock * sk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/filter.h>

int sk_reuseport_attach_filter(struct sock_fprog * fprog,struct sock * sk)
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

int smp_call_function_single_async(int cpu,struct __call_single_data * csd)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/smp.h>

void smp_send_stop(void)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sock_diag.h>

void sock_diag_broadcast_destroy(struct sock * sk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <net/sock.h>

void sock_edemux(struct sk_buff * skb)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/splice.h>

ssize_t splice_to_socket(struct pipe_inode_info * pipe,struct file * out,loff_t * ppos,size_t len,unsigned int flags)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/jump_label.h>

bool static_key_initialized;


#include <linux/fs.h>

int stream_open(struct inode * inode,struct file * filp)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk/sunxi-ng.h>

int sunxi_ccu_get_mmc_timing_mode(struct clk * clk)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clk/sunxi-ng.h>

int sunxi_ccu_set_mmc_timing_mode(struct clk * clk,bool new_mode)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/printk.h>

int suppress_printk;


#include <linux/sysctl.h>

const int sysctl_vals[] = {};


#include <linux/sysfs.h>

int sysfs_rename_dir_ns(struct kobject * kobj,const char * new_name,const void * new_ns)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sysfs.h>

int sysfs_rename_link_ns(struct kobject * kobj,struct kobject * targ,const char * old,const char * new,const void * new_ns)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/tcp.h>

struct sk_buff * tcp_get_timestamping_opt_stats(const struct sock * sk,const struct sk_buff * orig_skb,const struct sk_buff * ack_skb)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/clockchips.h>

void tick_broadcast(const struct cpumask * mask)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/fs.h>

int unregister_filesystem(struct file_system_type * fs)
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


#include <linux/reboot.h>

int unregister_restart_handler(struct notifier_block * nb)
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


#include <linux/mm.h>

int vm_insert_page(struct vm_area_struct * vma,unsigned long addr,struct page * page)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/mm.h>

struct page * vmalloc_to_page(const void * vmalloc_addr)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/filter.h>

int xdp_do_generic_redirect(struct net_device * dev,struct sk_buff * skb,struct xdp_buff * xdp,struct bpf_prog * xdp_prog)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/filter.h>

u32 xdp_master_redirect(struct xdp_buff * xdp)
{
	lx_emul_trace_and_stop(__func__);
}

