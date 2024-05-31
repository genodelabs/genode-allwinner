/**
 * \brief  Backend implementation for Linux
 * \author Josef Soentgen
 * \date   2022-06-14
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#ifndef _LX_DRM_H_
#define _LX_DRM_H_

#ifdef __cplusplus
extern "C" {
#endif

void *lx_drm_open(void);
void  lx_drm_close(void *);

void      lx_drm_gem_submit_ctx_id(void *, unsigned);
void      lx_drm_gem_submit_set_out_sync(void *, unsigned);
unsigned  lx_drm_gem_submit_bo_count(void const *);
unsigned *lx_drm_gem_submit_bo_handle(void *, unsigned);
bool      lx_drm_gem_submit_bo_read(void *, unsigned);
unsigned  lx_drm_gem_submit_out_sync(void const *);
unsigned  lx_drm_gem_submit_pipe(void const *);

int lx_drm_gem_close(void *, unsigned int);
int lx_drm_gem_flink(void *, unsigned int, unsigned int *);
int lx_drm_gem_open(void *, unsigned int, unsigned int *, unsigned long long *);

int lx_drm_gem_prime_handle_to_fd(void *, unsigned int, int *);
int lx_drm_gem_prime_fd_to_handle(void *, int, unsigned int *);

int lx_drm_ioctl_syncobj_create(void *, unsigned int *);
int lx_drm_ioctl_syncobj_destroy(void *, unsigned int);
int lx_drm_ioctl_syncobj_wait(void *, unsigned int);

int lx_drm_ioctl_lima_ctx_create(void *, unsigned int *);
int lx_drm_ioctl_lima_ctx_free(void *, unsigned int);
int lx_drm_ioctl_lima_gem_create(void *, unsigned, unsigned long, unsigned int *);
int lx_drm_ioctl_lima_gem_info(void *, unsigned int, unsigned int *, unsigned long long *);
int lx_drm_ioctl_lima_gem_param(void *, unsigned char, unsigned long long*);
int lx_drm_ioctl_lima_gem_wait(void *, unsigned int, unsigned int);
int lx_drm_ioctl_lima_gem_submit(void *, unsigned long);

#ifdef __cplusplus
}
#endif

#endif /* _LX_DRM_H_ */
