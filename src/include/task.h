/*
 * $File: task.h
 * $Date: Sun Dec 19 20:31:39 2010 +0800
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

#ifndef HEADER_TASK
#define HEADER_TASK

#include <common.h>

namespace Task
{
	typedef uint32_t pid_t;

	extern void init();

	// called by timer hook
	extern void schedule();


	extern pid_t fork();
	extern pid_t getpid();
	extern void exit(int status);

	// whether the current task is kernel code or user program
	extern bool is_kernel();

	// test whether the virtual address lies in kernel stack
	extern bool is_in_kernel_stack(uint32_t addr);

	// switch to user mode and continue execution at address @addr,
	// with esp set to @esp
	extern void switch_to_user_mode(uint32_t addr, uint32_t esp);
}

#endif // HEADER_TASK

