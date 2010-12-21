/*
 * $File: syscall.cpp
 * $Date: Tue Dec 21 16:17:11 2010 +0800
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

#include <syscall.h>
#include <descriptor_table.h>
#include <scio.h>
#include <task.h>

static int test(uint32_t arg)
{
	uint16_t cs;
	asm volatile ("mov %%cs, %0" : "=g"(cs));
	if (cs != 0x08)
		panic("%x %p", cs, &cs);
	Scio::push_color(Scio::GREEN);
	Scio::printf("called from user mode! arg=0x%x cs=0x%x &cs=%p\n", arg, cs, &cs);
	Scio::pop_color();
	return 1;
}

static uint32_t syscall_addr[] =
{
	(uint32_t)Scio::puts,
	(uint32_t)test,
	(uint32_t)Task::fork
};
const uint32_t NR_SYSCALLS = sizeof(syscall_addr) / sizeof(syscall_addr[0]);

static void syscall_handler(Isr_registers_t reg);


void Syscall::init()
{
	isr_register(0x80, syscall_handler);
}

void syscall_handler(Isr_registers_t reg)
{
	if (reg.eax >= NR_SYSCALLS)
		return;

	asm volatile
	(
		"push %1\n"
		"push %2\n"
		"push %3\n"
		"push %4\n"
		"push %5\n"
		"call *%6\n"
		"pop %%ebx\n"
		"pop %%ebx\n"
		"pop %%ebx\n"
		"pop %%ebx\n"
		"pop %%ebx\n"
		: "=a"(reg.eax)
		: "g"(reg.edi), "g"(reg.esi), "g"(reg.edx), "g"(reg.ecx), "g"(reg.ebx), "g"(syscall_addr[reg.eax])
	);
}

