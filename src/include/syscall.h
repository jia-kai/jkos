/*
 * $File: syscall.h
 * $Date: Thu Dec 09 11:52:45 2010 +0800
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

#ifndef HEADER_SYSCALL
#define HEADER_SYSCALL

namespace Syscall
{
	extern void init();

#define DEFN_SYSCALL1(num, fn, P0) \
	static inline int fn(P0 p0) \
	{ \
		int a; \
		asm volatile ("mov %1, %%eax\nint $0x80" : "=a"(a) : "i"(num), "b"((int)p0)); \
		return a; \
	}

	DEFN_SYSCALL1(0, puts, const char *);
}

#endif // HEADER_SYSCALL

