/*
 * \brief  Dummy definitions of Linux Kernel functions - handled manually
 * \author Josef Soentgen
 * \date   2022-02-09
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul.h>


#include <linux/types.h>

const u8 shipped_regdb_certs[] = { };
unsigned int shipped_regdb_certs_len = sizeof (shipped_regdb_certs);


#include <linux/filter.h>
#include <linux/jump_label.h> /* for DEFINE_STATIC_KEY_FALSE */

void bpf_prog_change_xdp(struct bpf_prog *prev_prog, struct bpf_prog *prog)
{
	lx_emul_trace(__func__);
}

DEFINE_STATIC_KEY_FALSE(bpf_stats_enabled_key);


asmlinkage __wsum csum_partial(const void * buff,int len,__wsum sum)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/proc_ns.h>

int proc_alloc_inum(unsigned int * inum)
{
	*inum = 1; /* according to linux/proc_ns.h without CONFIG_PROC_FS */
	return 0;
}


#include <net/net_namespace.h>

__init int net_sysctl_init(void)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/fs.h>

unsigned int get_next_ino(void)
{
	static unsigned int count = 0;
	return ++count;
}


#include <linux/netdevice.h>

int __init dev_proc_init(void)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/stringhash.h>

unsigned int full_name_hash(const void * salt,const char * name,unsigned int len)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/key.h>

struct key * keyring_alloc(const char * description,kuid_t uid,kgid_t gid,const struct cred * cred,key_perm_t perm,unsigned long flags,struct key_restriction * restrict_link,struct key * dest)
{
	static struct key _key;
	lx_emul_trace(__func__);
	return &_key;
}


#include <linux/kobject.h>

int kobject_uevent_env(struct kobject * kobj,enum kobject_action action,char * envp_ext[])
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/moduleparam.h>

void kernel_param_lock(struct module * mod)
{
	lx_emul_trace(__func__);
}


#include <linux/moduleparam.h>

void kernel_param_unlock(struct module * mod)
{
	lx_emul_trace(__func__);
}


#include <linux/pid.h>

void put_pid(struct pid * pid)
{
	lx_emul_trace(__func__);
}


#include <linux/filter.h>

int sk_filter_trim_cap(struct sock * sk,struct sk_buff * skb,unsigned int cap)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/capability.h>

bool file_ns_capable(const struct file * file,struct user_namespace * ns,int cap)
{
	lx_emul_trace(__func__);
	return true;
}


#include <linux/rcupdate.h>

void synchronize_rcu(void)
{
	lx_emul_trace(__func__);
}


#include <linux/skbuff.h>

void __skb_get_hash(struct sk_buff * skb)
{
	lx_emul_trace(__func__);
}


#include <linux/pid.h>

pid_t pid_vnr(struct pid * pid)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <linux/verification.h>

int verify_pkcs7_signature(const void *data, size_t len,
               const void *raw_pkcs7, size_t pkcs7_len,
               struct key *trusted_keys,
               enum key_being_used_for usage,
               int (*view_content)(void *ctx,
                           const void *data, size_t len,
                           size_t asn1hdrlen),
               void *ctx)
{
	return true;
}


#include <linux/net.h>

int net_ratelimit(void)
{
	lx_emul_trace(__func__);
	/* suppress */
	return 0;
}


#include <asm/smp.h>

void synchronize_rcu_expedited(void)
{
	lx_emul_trace(__func__);
}


#include <linux/netdevice.h>

int dev_ioctl(struct net * net,unsigned int cmd,struct ifreq * ifr,void __user * data,bool * need_copyout)
{
	lx_emul_trace_and_stop(__func__);
}


int get_option(char ** str,int * pint)
{
	lx_emul_trace_and_stop(__func__);
}


char * get_options(const char * str,int nints,int * ints)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/rcutree.h>

void rcu_barrier(void)
{
	lx_emul_trace(__func__);
}


#include <linux/skbuff.h>

bool __skb_flow_dissect(const struct net * net,const struct sk_buff * skb,struct flow_dissector * flow_dissector,void * target_container,const void * data,__be16 proto,int nhoff,int hlen,unsigned int flags)
{
	lx_emul_trace(__func__);
	return false;
}


#include <linux/dma-map-ops.h>

// XXX is this also valid on arm?
bool dma_default_coherent = false;


unsigned long net_rand_noise;


#include <linux/proc_fs.h>

struct proc_dir_entry { int dummy; };

struct proc_dir_entry * proc_create_net_data(const char * name,umode_t mode,struct proc_dir_entry * parent,const struct seq_operations * ops,unsigned int state_size,void * data)
{
	static struct proc_dir_entry _proc_dir_entry;
	lx_emul_trace(__func__);
	return &_proc_dir_entry;
}


struct proc_dir_entry * proc_create_seq_private(const char * name,umode_t mode,struct proc_dir_entry * parent,const struct seq_operations * ops,unsigned int state_size,void * data)
{
	static struct proc_dir_entry ret;
	lx_emul_trace(__func__);
	return &ret;
}


struct iwl_mvm;
struct ieee80211_vif;

void iwl_mvm_set_last_nonqos_seq(struct iwl_mvm *mvm,
                                 struct ieee80211_vif *vif)
{
	lx_emul_trace(__func__);
}


struct ieee80211_hw;
struct cfg80211_wowlan;

int __ieee80211_suspend(struct ieee80211_hw *hw, struct cfg80211_wowlan *wowlan)
{
	lx_emul_trace(__func__);
	return 0;
}


#include <net/addrconf.h>

int register_inet6addr_notifier(struct notifier_block * nb)
{
	lx_emul_trace(__func__);
	return -1;
}


#include <linux/inetdevice.h>

int register_inetaddr_notifier(struct notifier_block * nb)
{
	lx_emul_trace(__func__);
	return -1;
}


#include <net/addrconf.h>

int unregister_inet6addr_notifier(struct notifier_block * nb)
{
	lx_emul_trace(__func__);
	return -1;
}


#include <linux/inetdevice.h>

int unregister_inetaddr_notifier(struct notifier_block * nb)
{
	lx_emul_trace(__func__);
	return -1;
}


#include <linux/kernfs.h>

struct kernfs_node * kernfs_find_and_get_ns(struct kernfs_node * parent,const char * name,const void * ns)
{
	lx_emul_trace(__func__);
	return NULL;
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


#include <linux/gpio/consumer.h>

struct gpio_descs * __must_check devm_gpiod_get_array(struct device * dev,const char * con_id,enum gpiod_flags flags)
{
	lx_emul_trace(__func__);
	return ERR_PTR(-ENOSYS);
}


#include <linux/regulator/consumer.h>

int regulator_count_voltages(struct regulator * regulator)
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


#include <linux/regulator/consumer.h>

int regulator_set_voltage(struct regulator * regulator,int min_uV,int max_uV)
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
	printk("%s:%d con_id: '%s' idx: %u\n", __func__, __LINE__, con_id, idx);
	lx_emul_trace(__func__);
	return ERR_PTR(-ENOSYS);
}


#include <linux/netdevice.h>

void dev_load(struct net *net, const char *name)
{
	lx_emul_trace(__func__);
}


#include <linux/sched.h>

void sched_set_fifo_low(struct task_struct * p)
{
	lx_emul_trace(__func__);
}


#include <linux/signal.h>

void kernel_sigaction(int sig,__sighandler_t action)
{
	lx_emul_trace(__func__);
}


u8 get_random_u8(void)
{
	lx_emul_trace_and_stop(__func__);
}


/////////////////////////////////////////////////////////////
// XXX move below to lx_emul.c for a64
/////////////////////////////////////////////////////////////

#include <linux/regulator/consumer.h>

static int vmmc_regulator;

struct regulator * devm_regulator_get_optional(struct device * dev,const char * id)
{
	if (strcmp(id, "vqmmc") == 0)
		return ERR_PTR(-ENOSYS);

	if (strcmp(id, "vmmc") == 0)
		return (struct regulator*)&vmmc_regulator;

	return NULL;
}


int regulator_get_voltage(struct regulator * regulator)
{
	if (regulator == (struct regulator *)&vmmc_regulator)
		return 3300000;

	return 0;
}


#include <linux/sched.h>

void yield()
{
	lx_emul_task_schedule(false /* no block */);
}


#include <linux/filter.h>

DEFINE_STATIC_KEY_FALSE(bpf_master_redirect_enabled_key);
EXPORT_SYMBOL_GPL(bpf_master_redirect_enabled_key);


#include <net/gen_stats.h>

void gnet_stats_basic_sync_init(struct gnet_stats_basic_sync * b)
{
	lx_emul_trace(__func__);
}


/*
 * AFAICT this function can be a NOP as its intended purpose [1]
 * is not there in our case.
 *
 * [1] "This function is called when a page has been modified by the kernel.
 *      Mark it as dirty for later flushing when mapped in user space [...]"
 *      -- arm64/mm/flush.c
 *
 */
extern void flush_dcache_page(struct page * page);
void flush_dcache_page(struct page * page)
{
	lx_emul_trace(__func__);
}


#include <linux/mnt_idmapping.h>

struct mnt_idmap { unsigned dummy; };
struct mnt_idmap nop_mnt_idmap;


#include <linux/fs.h>

void inode_init_once(struct inode * inode)
{
	lx_emul_trace(__func__);
}
