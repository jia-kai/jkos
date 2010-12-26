/*
 * $File: task.h
 * $Date: Thu Dec 23 18:29:03 2010 +0800
 *
 * task scheduling and managing
 */
/*
This file is part of JKOS

Copyright (C) <2010>  Jiakai <jia.kai66@gmail.com>

JKOS is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

JKOS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with JKOS.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _HEADER_TASK_
#define _HEADER_TASK_

#include <common.h>
#include <signal.h>
#include <types.h>

namespace Task
{
	extern void init();

	// called by timer hook
	extern void schedule();


	extern pid_t fork();
	extern pid_t getpid();
	extern void exit(int status);

	// suspend the execution of task with pid @pid
	// the task will be automatically resumed on receiving any signal in @sig_wakeup
	extern int sleep(pid_t pid, const Sigset &sig_wakeup);

	extern int wakeup(pid_t pid);


	// whether the current task is kernel code or user program
	extern bool is_kernel();

	// switch to user mode and continue execution at address @addr,
	// with esp set to @esp
	extern void switch_to_user_mode(uint32_t addr, uint32_t esp);
}

#endif // _HEADER_TASK_

