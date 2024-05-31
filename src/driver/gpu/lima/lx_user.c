/*
 * \brief  Post kernel userland activity
 * \author Stefan Kalkowski
 * \date   2021-07-14
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul/task.h>
#include <lx_user/init.h>
#include <lx_user/io.h>

#include <linux/sched/task.h>


struct task_struct * lx_user_task;
void               * lx_user_task_args;

extern int lx_user_task_func(void *p);


void lx_user_handle_io(void) { }

void lx_user_init(void)
{
	int pid = kernel_thread(lx_user_task_func, lx_user_task_args,
	                        CLONE_FS | CLONE_FILES);
	lx_user_task = find_task_by_pid_ns(pid, NULL);
}


struct task_struct *lx_user_new_gpu_task(int (*func)(void*), void *args)
{
	int pid = kernel_thread(func, args, CLONE_FS | CLONE_FILES);
	return find_task_by_pid_ns(pid, NULL);
}


void lx_user_destroy_gpu_task(struct task_struct *task)
{
	if (task != current) {
		printk("%s: task: %px is not current: %px\n", __func__,
		       task, current);
		return;
	}

	do_exit(0);
}
