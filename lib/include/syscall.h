/*
 * $File: syscall.h
 * $Date: Wed Dec 29 19:52:52 2010 +0800
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

#ifndef _HEADER_SYSCALL_
#define _HEADER_SYSCALL_

typedef int pid_t;
class Sigset;

#define DEFN_SYSCALL0(num, fn, ret_t) \
static inline ret_t fn() \
{ \
	ret_t a; \
	asm volatile ("int $0x80" : "=a"(a) : "a"(num)); \
	return a; \
}

#define DEFN_SYSCALL1(num, fn, ret_t, P0) \
static inline ret_t fn(P0 p0) \
{ \
	ret_t a; \
	asm volatile ("int $0x80" : "=a"(a) : "a"(num), "b"((int)p0)); \
	return a; \
}

#define DEFN_SYSCALL2(num, fn, ret_t, P0, P1) \
static inline ret_t fn(P0 p0, P1 p1) \
{ \
	ret_t a; \
	asm volatile ("int $0x80" : "=a"(a) : "a"(num), "b"((int)p0), "c"((int)p1)); \
	return a; \
}

DEFN_SYSCALL1(0, sys_puts, int, const char *);
DEFN_SYSCALL0(1, sys_fork, pid_t);
DEFN_SYSCALL0(2, sys_getpid, pid_t);

static inline pid_t sys_sleep(pid_t pid, const Sigset &sig_wakeup)
{
	pid_t a;
	asm volatile ("int $0x80" : "=a"(a) : "a"(3), "b"((int)pid), "c"((int)&sig_wakeup));
	return a;
}

DEFN_SYSCALL1(4, sys_wakeup, pid_t, pid_t);

#endif
