/*
 * $File: task.cpp
 * $Date: Wed Dec 08 20:17:17 2010 +0800
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
#include <asm.h>

using namespace Task;

struct Task_t
{
	Task::pid_t id;
	uint32_t esp, ebp, eip;
	Page::Directory_t *page_dir;

	volatile Task_t *next;
	Task_t(Page::Directory_t *dir);
};

struct Task_queue
{
	// cycle queue
	volatile Task_t *head, *tail;

	// return NULL if the queue is empty
	volatile Task_t* pop();

	void append(volatile Task_t *task);
};

const uint32_t
	INIT_STACK_SIZE = 1024 * 16,
	STACK_POS = 0xA0000000u, STACK_SIZE = 16 * 1024;

// defined in loader.s
extern "C" uint32_t initial_stack_pointer;

static volatile Task_t *current_task;



namespace Pid_allocator
{
	static pid_t next;
	static pid_t get();
	// XXX: not used
	void free(pid_t pid);
}

namespace Queue
{
	static Task_queue ready;
	// currently only ready queue is used
}

// move the stack out of kernel pages
// (to avoid being linked when cloing page directories)
static void move_stack();

// defined in misc.s
extern "C" uint32_t read_eip();

void Task::init()
{
	move_stack();

	// initialise the first task (kernel task)
	current_task = new Task_t(Page::current_page_dir);
	Queue::ready.append(current_task);

	MSG_INFO("tasking initialized");
}

pid_t Task::fork()
{
	asm volatile ("cli");

	uint32_t eip;
	volatile Task_t *par_task = current_task, *child;

	child = new Task_t(Page::clone_directory(Page::current_page_dir));
	Queue::ready.append(child);

	eip = read_eip();

	// we could be either the parent task or child task
	if (current_task == par_task)
	{
		asm volatile
		(
			"mov %%esp, %0\n"
			"mov %%ebp, %1\n"
			: "=g"(child->esp), "=g"(child->ebp)
		);
		child->eip = eip;

		asm volatile ("sti");
		return child->id;
	}

	//  now we are the child, return 0 by convention
	return 0;
}

void Task::schedule()
{
	uint32_t esp, ebp, eip;
	asm volatile
	(
		"mov %%esp, %0\n"
		"mov %%ebp, %1\n"
		: "=g"(esp), "=g"(ebp)
	);
	eip = read_eip();

	if (eip == 0xFFFFFFFF)
		return; // we have just switched to this task

	current_task->esp = esp;
	current_task->ebp = ebp;
	current_task->eip = eip;

	current_task = current_task->next;

	esp = current_task->esp;
	ebp = current_task->ebp;
	eip = current_task->eip;

	Page::current_page_dir = current_task->page_dir;
	asm volatile
	(
		"cli\n"
		"mov %0, %%ecx\n"
		"mov %1, %%esp\n"
		"mov %2, %%ebp\n" // IMPORTANT: ebp must be changed last
		"mov %3, %%cr3\n"
		"mov $0xFFFFFFFF, %%eax\n"
			// so after jump, we can replace the return value of read_eip() above
		"sti\n"
		"jmp *%%ecx"
		: : "g"(eip), "g"(esp), "g"(ebp), "g"(Page::current_page_dir->phyaddr)
	);
}

pid_t Task::getpid()
{
	return current_task->id;
}

volatile Task_t* Task_queue::pop()
{
	if (!head)
		return NULL;
	volatile Task_t *ret = head;
	if (head == tail)
		head = tail = NULL;
	else
		head = head->next;
	return ret;
}

void Task_queue::append(volatile Task_t *task)
{
	if (!head)
		(head = tail = task)->next = task;
	else
		(tail = tail->next = task)->next = head;
}

Task_t::Task_t(Page::Directory_t *dir) :
	id(Pid_allocator::get()), esp(0), ebp(0), eip(0),
	page_dir(dir), next(NULL)
{
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

pid_t Pid_allocator::get()
{
	return next ++;
}

void Pid_allocator::free(pid_t)
{
}

bool Task::is_kernel()
{
	return true;
}

bool Task::is_in_kernel_stack(uint32_t addr)
{
	return addr >= STACK_POS - STACK_SIZE && addr < STACK_POS;
}

void Task::switch_to_user_mode(uint32_t addr)
{
	asm volatile
	(
		"cli\n"
		"mov %0, %%ax\n"		// set user mode data selector
		"mov %%ax, %%ds\n"
		"mov %%ax, %%es\n"
		"mov %%ax, %%fs\n"
		"mov %%ax, %%gs\n"

		"mov %%esp, %%eax\n"
		"pushl %0\n"
		"pushl %%eax\n"
		"pushf\n"

		"pop %%eax\n"
		"or $0x200, %%eax\n"	// set the IF flag
		"push %%eax\n"

		"pushl %1\n"			// set user mode code selector
		"pushl %2\n"
		"iret\n" : : "i"(USER_DATA_SELECTOR | 0x3), "i"(USER_CODE_SELECTOR | 0x3), "g"(addr)
	);
}

