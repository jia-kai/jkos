/*
 * $File: task.cpp
 * $Date: Sun Dec 05 20:03:52 2010 +0800
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

#include <task.h>
#include <common.h>
#include <page.h>
#include <scio.h>
#include <lib/cstring.h>

struct Task_t
{
	Task::pid_t id;
	uint32_t esp, ebp, eip;
	Page::Directory_t *page_dir;
};

const uint32_t
	INIT_STACK_SIZE = 1024 * 16,
	STACK_POS = 0xA0000000u, STACK_SIZE = 16 * 1024;

// defined in loader.s
extern "C" uint32_t initial_stack_pointer;

// move the stack out of kernel pages
// (to avoid being linked when cloing page directories)
static void move_stack();

void Task::init()
{
	asm volatile
	(
		"pushf\n"
		"cli\n"
	);
	move_stack();
	asm volatile ("popf");
	MSG_INFO("tasking initialized");
}

void move_stack()
{
	static uint32_t i, old_esp, old_ebp, dest, size,
					offset, new_esp, new_ebp, tmp;
	// we use static variables to avoid corruption
	dest = STACK_POS;
	size = STACK_SIZE;
	for (i = dest - 1; i >= dest - size; i -= 0x1000)
	{
		Page::current_page_dir->get_page(i, true)->alloc(true, true);
		// usermode !
		// **PERMISSION_CONTROL**
		Page::invlpg(i);
	}

	asm volatile
	(
		"mov %%esp, %0\n"
		"mov %%ebp, %1\n"
		: "=g"(old_esp), "=g"(old_ebp)
	);

	offset = dest - initial_stack_pointer;
	new_esp = old_esp + offset;
	new_ebp = old_ebp + offset;

	memcpy((void*)new_esp, (void*)old_esp, initial_stack_pointer - old_esp);

	for (i = dest - 4; i > new_esp; i -= 4)
	{
		tmp = *(uint32_t*)i;
		if (tmp < initial_stack_pointer && tmp > old_esp)
			(*(uint32_t*)i) += offset;
	}

	asm volatile
	(
		"mov %0, %%esp\n"
		"mov %1, %%ebp\n"
		: : "g"(new_esp), "g"(new_ebp)
	);
}

