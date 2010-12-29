/*
 * $File: syscall.cpp
 * $Date: Wed Dec 29 19:41:15 2010 +0800
 *
 * interface for implementing system calls
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

#include <asm.h>
#include <klog.h>
#include <task.h>

extern "C" uint32_t syscall_func_addr[NR_SYSCALLS];

static int sys_sleep(pid_t pid, const Sigset *sig_wakeup);

uint32_t syscall_func_addr[NR_SYSCALLS] =
{
	(uint32_t)Klog::puts,
	(uint32_t)Task::fork,
	(uint32_t)Task::getpid,
	(uint32_t)sys_sleep,
	(uint32_t)Task::wakeup
};

static int sys_sleep(pid_t pid, const Sigset *sig_wakeup)
{
	return Task::sleep(pid, *sig_wakeup);
}

