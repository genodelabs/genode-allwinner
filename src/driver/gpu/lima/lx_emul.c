/*
 * \brief  Gpu framebuffer driver
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


#include <linux/mount.h>
#include <linux/fs.h>
#include <linux/slab.h>

struct vfsmount * kern_mount(struct file_system_type * type)
{
	struct vfsmount *m;

	m = kzalloc(sizeof (struct vfsmount), 0);
	if (!m)
		return (struct vfsmount*)ERR_PTR(-ENOMEM);

	return m;
}


#include <linux/fs.h>
#include <linux/mount.h>

int simple_pin_fs(struct file_system_type * type,struct vfsmount ** mount,int * count)
{
    *mount = kzalloc(sizeof(struct vfsmount), GFP_KERNEL);
    return 0;
}


#include <linux/fs.h>

static unsigned long _get_next_ino(void)
{
    static unsigned long count = 0;
    return ++count;
}


struct inode *alloc_anon_inode(struct super_block *s)
{
    struct inode *inode;

    inode = kzalloc(sizeof (struct inode), 0);
    if (!inode) {
        return (struct inode*)ERR_PTR(-ENOMEM);
    }

    inode->i_ino = _get_next_ino();

    return inode;
}


void inode_set_bytes(struct inode *inode, loff_t bytes)
{
	if (!inode)
		return;

	// inode->i_blocks = bytes >> 9;
	// inode->i_bytes = bytes & 511;
	inode->i_bytes = bytes;
}


#include <linux/file.h>

struct file * alloc_file_pseudo(struct inode * inode,
                                struct vfsmount * mnt,
                                const char * name,
                                int flags,
                                const struct file_operations * fops)
{
	struct file *file;
	struct dentry *dentry;

	file = kzalloc(sizeof(struct file), 0);
	if (!file)
		goto err_file;

	dentry = kzalloc(sizeof(struct dentry), 0);
	if (!dentry)
		goto err_dentry;

	atomic_long_set(&file->f_count, 1);
	file->f_op    = fops;
	file->f_inode = inode;
	file->f_path.dentry = dentry;
	return file;

err_dentry:
	kfree(file);
err_file:
	return (struct file*)ERR_PTR(-ENOMEM);
}


int get_unused_fd_flags(unsigned flags)
{
	lx_emul_trace_and_stop(__func__);
	return -1;
}


void fd_install(unsigned int fd,struct file * file)
{
	lx_emul_trace_and_stop(__func__);
}


struct file * fget(unsigned int fd)
{
	lx_emul_trace_and_stop(__func__);
	return NULL;
}


void lx_emul_fput(unsigned int fd)
{
	lx_emul_trace_and_stop(__func__);
}


#include <linux/sched.h>

char * __get_task_comm(char * buf,size_t buf_size,struct task_struct * tsk)
{
	buf[0] = 0;
	return buf;
}


/*********
 ** DRM **
 *********/

#include "emul.h"
#include "lx_drm.h"

#include <linux/fs.h>

static struct file_operations const *_drm_fops;

int __register_chrdev(unsigned int major, unsigned int baseminor,
                      unsigned int count, char const * name,
                      struct file_operations const * fops)
{
	_drm_fops = fops;
	return 0;
}

#include <drm/drm_device.h>

extern struct drm_device *lx_drm_dev;

struct drm_device *_lx_drm_device;


#include <drm/drm_file.h>

struct lx_drm_private
{
	struct file  *file;
	struct inode *inode;
};


#include <drm/drm_ioctl.h>
#include <linux/kdev_t.h>

void *lx_drm_open(void)
{
	int err;
	struct lx_drm_private *lx_drm_prv;

	if (!_drm_fops || !_drm_fops->open)
		return NULL;

	lx_drm_prv = (struct lx_drm_private*)kzalloc(sizeof (struct lx_drm_private), 0);
	if (!lx_drm_prv)
		return NULL;

	lx_drm_prv->inode = alloc_anon_inode(NULL);
	if (!lx_drm_prv->inode)
		goto free_session;

	lx_drm_prv->inode->i_rdev = MKDEV(DRM_MAJOR, DRM_MINOR_PRIMARY);

	lx_drm_prv->file = (struct file*)kzalloc(sizeof (struct file), 0);
	if (!lx_drm_prv->file)
		goto free_inode;

	err = _drm_fops->open(lx_drm_prv->inode, lx_drm_prv->file);
	if (err)
		goto free_file;

	return lx_drm_prv;

free_file:
	kfree(lx_drm_prv->file);
free_inode:
	kfree(lx_drm_prv->inode);
free_session:
	kfree(lx_drm_prv);
	return NULL;
}


#include <drm/drm_drv.h>
#include <../drivers/gpu/drm/drm_internal.h>

void lx_drm_close(void *p)
{
	struct lx_drm_private *lx_drm_prv;

	if (!p || !_drm_fops || !_drm_fops->release) {
		return;
	}

	lx_drm_prv = (struct lx_drm_private*)p;

	(void)_drm_fops->release(lx_drm_prv->inode, lx_drm_prv->file);

	kfree(lx_drm_prv->inode);
	kfree(lx_drm_prv->file);
	kfree(lx_drm_prv);
}


#include <drm/drm_ioctl.h>
#include <uapi/drm/drm.h>
#include <uapi/drm/lima_drm.h>

void lx_drm_gem_submit_ctx_id(void *p, unsigned id)
{
	struct drm_lima_gem_submit * const submit =
		(struct drm_lima_gem_submit *)p;
	submit->ctx = id;
}


void lx_drm_gem_submit_set_out_sync(void *p, unsigned id)
{
	struct drm_lima_gem_submit * const submit =
		(struct drm_lima_gem_submit *)p;
	submit->out_sync = id;
}


unsigned lx_drm_gem_submit_bo_count(void const *p)
{
	struct drm_lima_gem_submit const * const submit =
		(struct drm_lima_gem_submit const*)p;
	return submit->nr_bos;
}


unsigned *lx_drm_gem_submit_bo_handle(void *p, unsigned index)
{
	struct drm_lima_gem_submit * const submit =
		(struct drm_lima_gem_submit*)p;

	struct drm_lima_gem_submit_bo *bos =
		(struct drm_lima_gem_submit_bo*)(submit->bos + (unsigned long)submit);

	struct drm_lima_gem_submit_bo * const bo = &bos[index];
	return &bo->handle;
}


bool lx_drm_gem_submit_bo_read(void *p, unsigned index)
{
	struct drm_lima_gem_submit * const submit =
		(struct drm_lima_gem_submit*)p;

	struct drm_lima_gem_submit_bo *bos =
		(struct drm_lima_gem_submit_bo*)(submit->bos + (unsigned long)submit);

	struct drm_lima_gem_submit_bo * const bo = &bos[index];
	return bo->flags & LIMA_SUBMIT_BO_READ;
}


unsigned lx_drm_gem_submit_out_sync(void const *p)
{
	struct drm_lima_gem_submit const * const submit =
		(struct drm_lima_gem_submit const*)p;
	return submit->out_sync;
}


unsigned lx_drm_gem_submit_pipe(void const *p)
{
	struct drm_lima_gem_submit const * const submit =
		(struct drm_lima_gem_submit const*)p;
	return submit->pipe;
}


static void lx_drm_gem_submit_in(struct drm_lima_gem_submit *submit)
{
	submit->bos   += (unsigned long)submit;
	submit->frame += (unsigned long)submit;
}


static void lx_drm_version_in(struct drm_version *version)
{
	/* set proper pointer value from offset */
	version->name += (unsigned long)version;
	version->date += (unsigned long)version;
	version->desc += (unsigned long)version;
}


static void lx_drm_version_out(struct drm_version *version)
{
	/* set proper offset value from pointer */
	version->name -= (unsigned long)version;
	version->date -= (unsigned long)version;
	version->desc -= (unsigned long)version;
}


static int lx_drm_in(unsigned int cmd, unsigned long arg)
{
	unsigned int const nr = DRM_IOCTL_NR(cmd);
	bool const is_driver_ioctl =
		nr >= DRM_COMMAND_BASE && nr < DRM_COMMAND_END;

	if (is_driver_ioctl) {
		unsigned const int dnr = nr - DRM_COMMAND_BASE;

		switch (dnr) {
		case DRM_LIMA_GEM_SUBMIT:
			lx_drm_gem_submit_in((struct drm_lima_gem_submit*)arg);
			break;
		default:
			break;
		}
	} else {
		switch (nr) {
		case DRM_IOCTL_NR(DRM_IOCTL_VERSION):
			lx_drm_version_in((struct drm_version*)arg);
			break;
		default:
			break;
		}
	}
	return 0;
}


static int lx_drm_out(unsigned int cmd, unsigned long arg)
{
	unsigned int const nr = DRM_IOCTL_NR(cmd);
	bool const is_driver_ioctl =
		nr >= DRM_COMMAND_BASE && nr < DRM_COMMAND_END;

	if (is_driver_ioctl) {
		unsigned const int dnr = nr - DRM_COMMAND_BASE;

		switch (dnr) {
		default:
			break;
		}
	} else {
		switch (nr) {
		case DRM_IOCTL_NR(DRM_IOCTL_VERSION):
			lx_drm_version_out((struct drm_version*)arg);
			break;
		default:
			break;
		}
	}
	return 0;
}

int lx_drm_ioctl(void *p, unsigned int cmd, unsigned long arg)
{
	struct lx_drm_private *lx_drm_prv;
	int res;

	lx_drm_prv = (struct lx_drm_private*)p;

	if (!lx_drm_prv) {
		return -1;
	}

	if (cmd & IOC_IN) {
		lx_drm_in(cmd, arg);
	}
	res = drm_ioctl(lx_drm_prv->file, cmd, arg);
	if (cmd & IOC_OUT) {
		lx_drm_out(cmd, arg);
	}
	return res;
}


int lx_drm_ioctl_syncobj_create(void *p , unsigned int *handle)
{
	int err;
	struct lx_drm_private *lx_drm_prv;

	struct drm_syncobj_create req = {
		.handle = 0,
		.flags  = DRM_SYNCOBJ_CREATE_SIGNALED,
	};

	lx_drm_prv = (struct lx_drm_private*)p;

	err = drm_ioctl(lx_drm_prv->file, DRM_IOCTL_SYNCOBJ_CREATE,
	                (unsigned long)&req);
	if (err)
		return -1;

	*handle = req.handle;
	return 0;
}


int lx_drm_ioctl_syncobj_destroy(void *p, unsigned int handle)
{
	struct lx_drm_private *lx_drm_prv;

	struct drm_syncobj_destroy req = {
		.handle = handle,
		.pad    = 0,
	};

	lx_drm_prv = (struct lx_drm_private*)p;

	return drm_ioctl(lx_drm_prv->file, DRM_IOCTL_SYNCOBJ_DESTROY,
	                 (unsigned long)&req);
}


int lx_drm_ioctl_syncobj_wait(void *p, unsigned int handle)
{
	int err;
	struct lx_drm_private *lx_drm_prv;

	enum { COUNT_HANDLES = 1 };
	uint64_t handles[COUNT_HANDLES] = { handle };

	struct drm_syncobj_wait req = {
		.handles        = (uint64_t)handles,
		.timeout_nsec   = 4611686018427387904, // excessive large timeout
		.count_handles  = COUNT_HANDLES,
		.flags          = 0,
		.first_signaled = 0,
		.pad            = 0,
	};

	lx_drm_prv = (struct lx_drm_private*)p;

	err = drm_ioctl(lx_drm_prv->file, DRM_IOCTL_SYNCOBJ_WAIT,
	                (unsigned long)&req);
	if (err == -ENOENT)
		err = 0;
	if (err == -ETIME)
		err = 1;

	return err;
}


/*
 * The next functions are used by the Gpu lx_drm_prv to perform I/O controls.
 */

int lx_drm_ioctl_lima_ctx_create(void *lx_drm_prv,
                                 unsigned int *id)
{
	int err;
	struct drm_lima_ctx_create req = {
		.id   = 0,
		._pad = 0,
	};

	err = lx_drm_ioctl(lx_drm_prv, DRM_IOCTL_LIMA_CTX_CREATE,
	                   (unsigned long)&req);
	if (err)
		return -1;

	*id = req.id;
	return 0;
}


int lx_drm_ioctl_lima_ctx_free(void *lx_drm_prv,
                                 unsigned int id)
{
	int err;
	struct drm_lima_ctx_create req = {
		.id   = id,
		._pad = 0,
	};

	err = lx_drm_ioctl(lx_drm_prv, DRM_IOCTL_LIMA_CTX_FREE,
	                   (unsigned long)&req);
	return err;
}


int lx_drm_ioctl_lima_gem_param(void *lx_drm_prv,
                                unsigned char param,
                                unsigned long long *value)
{
	int err;
	struct drm_lima_get_param req = {
		.param = param,
		.pad   = 0,
		.value = 0,
	};

	err = lx_drm_ioctl(lx_drm_prv, DRM_IOCTL_LIMA_GET_PARAM,
	                   (unsigned long)&req);
	if (err)
		return -1;

	*value = req.value;
	return 0;
}


int lx_drm_ioctl_lima_gem_submit(void *lx_drm_prv, unsigned long arg)
{
	int err;
	err = lx_drm_ioctl(lx_drm_prv, DRM_IOCTL_LIMA_GEM_SUBMIT, arg);
	return err;
}


int lx_drm_ioctl_lima_gem_wait(void *lx_drm_prv, unsigned int handle,
                               unsigned int op)
{
	int err;

	struct drm_lima_gem_wait req = {
		.handle     = handle,
		.op         = op,
		.timeout_ns = 4611686018427387904, // excessive large timeout
	};

	err = lx_drm_ioctl(lx_drm_prv, DRM_IOCTL_LIMA_GEM_WAIT,
	                   (unsigned long) &req);
	return err;
}


int lx_drm_ioctl_lima_gem_create(void *lx_drm_prv,
                                 unsigned va,
                                 unsigned long size,
                                 unsigned int *handle)
{
	int err;
	struct drm_lima_gem_create req = {
		.size   = size,
		.flags  = LIMA_BO_FLAG_FORCE_VA, // FORCE va
		.handle = 0,
		.va     = va,
	};

	err = lx_drm_ioctl(lx_drm_prv, DRM_IOCTL_LIMA_GEM_CREATE,
	                   (unsigned long)&req);
	if (err)
		return -1;

	*handle = req.handle;
	return 0;
}


int lx_drm_ioctl_lima_gem_info(void *lx_drm_prv,
                               unsigned int handle,
                               unsigned int       *va,
                               unsigned long long *offset)
{
	int err;
	struct drm_lima_gem_info req = {
		.handle = handle,
		.va     = 0,
		.offset = 0,
	};

	err = lx_drm_ioctl(lx_drm_prv, DRM_IOCTL_LIMA_GEM_INFO,
	                   (unsigned long)&req);
	if (err)
		return -1;

	*va     = req.va;
	*offset = req.offset;
	return 0;
}


int lx_drm_gem_close(void *lx_drm_prv, unsigned int handle)
{
	int err;
	struct drm_gem_close req = {
		.handle = handle,
	};

	err = lx_drm_ioctl(lx_drm_prv, DRM_IOCTL_GEM_CLOSE, (unsigned long)&req);
	if (err) {
		return -1;
	}

	return 0;
}


int lx_drm_gem_flink(void *lx_drm_prv, unsigned int handle,
                          unsigned int *name)
{
	int err;
	struct drm_gem_flink req = {
		.handle = handle,
	};

	err = lx_drm_ioctl(lx_drm_prv, DRM_IOCTL_GEM_FLINK, (unsigned long)&req);
	if (err) {
		return -1;
	}

	*name = req.name;

	return 0;
}


int lx_drm_gem_open(void *lx_drm_prv, unsigned int name,
                     unsigned int *handle, unsigned long long *size)
{
	int err;
	struct drm_gem_open req = {
		.name = name,
	};

	err = lx_drm_ioctl(lx_drm_prv, DRM_IOCTL_GEM_OPEN, (unsigned long)&req);
	if (err) {
		return -1;
	}

	*handle = req.handle;
	*size   = req.size;

	return 0;
}


#include <drm/drm_gem.h>
#include <drm/drm_vma_manager.h>
#include <lx_emul/shmem_file.h>

void *genode_lookup_mapping_from_offset(void *p,
                                        unsigned long offset,
                                        unsigned long size)
{
	struct lx_drm_private *lx_drm_prv;
	struct file *file;
	struct drm_file *drm_file;
	struct drm_device *dev;
	struct drm_vma_offset_manager *mgr;
	struct drm_vma_offset_node *node;

	lx_drm_prv = (struct lx_drm_private*)p;
	if (!lx_drm_prv)
		return NULL;

	file = lx_drm_prv->file;
	if (!file)
		return NULL;

	drm_file = file->private_data;
	if (!drm_file)
		return NULL;

	dev = drm_file->minor->dev;
	if (!dev)
		return NULL;

	mgr = dev->vma_offset_manager;

	drm_vma_offset_lock_lookup(mgr);
	node = drm_vma_offset_lookup_locked(mgr, offset / PAGE_SIZE,
	                                    size / PAGE_SIZE);
	drm_vma_offset_unlock_lookup(mgr);

	if (node) {
		struct drm_gem_object *obj =
			container_of(node, struct drm_gem_object, vma_node);
		if (obj) {
			struct file *f = obj->filp;
			if (f) {
				struct address_space *mapping = f->f_mapping;
				if (mapping) {
					struct shmem_file_buffer *private_data =
						mapping->private_data;
					if (private_data) {
						return private_data->addr;
					}
				}
			}
		}
	}
	return NULL;
}


#include <linux/dma-mapping.h>

int dma_map_sgtable(struct device *dev, struct sg_table *sgt,
                    enum dma_data_direction dir, unsigned long attrs)
{
	int nents = dma_map_sg_attrs(dev, sgt->sgl, sgt->orig_nents, dir, attrs);
	if (nents < 0)
		return nents;
	sgt->nents = nents;
	return 0;
}
