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


struct task_struct * _lx_user_task;
void *lx_user_task_args;
extern int run_lx_user_task(void *p);


void lx_user_handle_io(void) { }


void lx_user_init(void)
{
	int pid = kernel_thread(run_lx_user_task, lx_user_task_args,
	                        CLONE_FS | CLONE_FILES);
	_lx_user_task = find_task_by_pid_ns(pid, NULL);
}


extern int gpu_task_function(void *p);

struct task_struct *lx_user_new_gpu_task(void *args)
{
	int pid = kernel_thread(gpu_task_function, args,
	                        CLONE_FS | CLONE_FILES);
	return find_task_by_pid_ns(pid, NULL);
}
