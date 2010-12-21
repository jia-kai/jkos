/*
 * $File: task.cpp
 * $Date: Tue Dec 21 15:04:00 2010 +0800
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
#include <asm.h>
#include <kheap.h>
#include <descriptor_table.h>
#include <lib/cstring.h>
#include <lib/rbtree.h>

using namespace Task;

// struct definitions
struct Task_t;
struct Pair_id_task
{
	pid_t id;
	volatile Task_t *task;
	inline bool operator < (const Pair_id_task &p) const
	{ return id < p.id; }
};
typedef Rbt<Pair_id_task> Rbt_id_task;

struct Task_t
{
	pid_t id;
	uint32_t esp, ebp, eip;
	Page::Directory_t *page_dir;

	int errno; // saved error number of current task

	volatile Task_t
		*par, // parent task
		*queue_next, *queue_prev; // next and previuos task in the task queue

	Task_t(Page::Directory_t *dir);

	static void *rbt_alloc();
	static void rbt_free(void *);

private:
	uint8_t rbt_node_mem[sizeof(Rbt_id_task::Node) + 1];
	static volatile Task_t *latest_task;
};
 
struct Task_queue
{
	// task queue is a cycle queue

	volatile Task_t *ptr;
	// pointer to an arbitrary task in this queue, or NULL iff the queue is empty

	void insert(volatile Task_t *task);

	// remove a task from this queue
	// @task must be in this queue (no check performed!)
	void remove(volatile Task_t *task);
};



// variable definitions
static bool switch_to_user_mode_called = false;
static volatile Task_t *current_task;
static Rbt_id_task rbt_id2task(Task_t::rbt_alloc, Task_t::rbt_free);
volatile Task_t *Task_t::latest_task;
extern "C" uint32_t initial_stack_pointer; // defined in loader.s
namespace Queue
{
	Task_queue
		running,
		stopped, // stopped, both interruptible and uninterruptible
		zombie;
}


// function declarations

// move the stack out of kernel pages
// (to avoid being linked when cloing page directories)
static void move_stack();

static inline volatile Task_t* get_next_task();
static inline volatile Task_t* id2task(pid_t pid);
static inline pid_t get_next_pid();

// defined in misc.s
extern "C" uint32_t read_eip();

#define CLI_SAVE_EFLAGS(_var_) \
asm volatile \
( \
	"pushf\n" \
	"cli\n" \
	"popl %0" \
  : "=g"(_var_) \
)


// function implementations

void Task::init()
{
	move_stack();

	// initialise the first task (kernel task)
	current_task = new Task_t(Page::current_page_dir);
	Queue::running.insert(current_task);

	schedule();

	MSG_INFO("tasking initialized");
}

pid_t Task::fork()
{
	uint32_t old_eflags;
	CLI_SAVE_EFLAGS(old_eflags);

	uint32_t eip;
	volatile Task_t *par_task = current_task, *child;
	pid_t ret = 0;

	child = new Task_t(Page::clone_directory(Page::current_page_dir));
	child->par = par_task;
	Queue::running.insert(child);

	eip = read_eip();

	if (current_task == par_task)
	{
		// we are the parent task
		asm volatile
		(
			"mov %%esp, %0\n"
			"mov %%ebp, %1\n"
			: "=g"(child->esp), "=g"(child->ebp)
		);
		child->eip = eip;
		// so upon next scheduling, child will be started executing at
		// read_eip() above

		ret = child->id;

	}

	asm volatile // restore interrupt state
	(
		"pushl %0\n"
		"popf\n"
		: : "g"(old_eflags)
	); 

	return ret;
}

void Task::schedule()
{
	uint32_t old_eflags;
	CLI_SAVE_EFLAGS(old_eflags);

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

	current_task = get_next_task();

	esp = current_task->esp;
	ebp = current_task->ebp;
	eip = current_task->eip;

	Page::current_page_dir = current_task->page_dir;
	asm volatile
	(
		"movl %[old_eflags], %%ebx\n"
		"movl %0, %%eax\n"
		"movl %1, %%ecx\n"
		"movl %2, %%esp\n"
		"movl %3, %%cr3\n"
		"movl %%eax, %%ebp\n" // IMPORTANT: ebp must be changed last
		"movl $0xFFFFFFFF, %%eax\n"
			// so after jump, we can replace the return value of read_eip() above
		"pushl %%ebx\n"
		"popf\n"
		"jmp *%%ecx"
		: : "g"(ebp), "g"(eip), "g"(esp), "g"(Page::current_page_dir->phyaddr), [old_eflags]"g"(old_eflags)
		: "eax", "ebx", "ecx"
	);
}

pid_t Task::getpid()
{
	return current_task->id;
}

void Task::exit(int status)
{
	uint32_t old_eflags;
	CLI_SAVE_EFLAGS(old_eflags);
	status = old_eflags;
}


Task_t::Task_t(Page::Directory_t *dir) :
	id(get_next_pid()), esp(0), ebp(0), eip(0),
	page_dir(dir), errno(0),
	par(NULL), queue_next(NULL), queue_prev(NULL)
{
	Task_t::latest_task = this;
	Pair_id_task val;
	val.id = this->id;
	val.task = this;
	rbt_id2task.insert(val);
}

void Task_queue::insert(volatile Task_t *task)
{
	if (!ptr)
	{
		ptr = task;
		task->queue_prev = task->queue_next = task;
	}
	else
	{
		task->queue_next = ptr->queue_next;
		task->queue_next->queue_prev = task;
		ptr->queue_next = task;
		task->queue_prev = ptr;
	}
}

void Task_queue::remove(volatile Task_t *task)
{
	if (ptr->queue_next == ptr)
	{
		kassert(ptr == task);
		ptr = NULL;
	} else
	{
		if (ptr == task)
			ptr = ptr->queue_next;
		task->queue_prev->queue_next = task->queue_next;
		task->queue_next->queue_prev = task->queue_prev;
		task->queue_prev = task->queue_next = NULL;
	}
}

pid_t get_next_pid()
{
	static pid_t next;
	while (id2task(next))
		next ++;
	return next ++;
}

bool Task::is_kernel()
{
	return !switch_to_user_mode_called;
}

void Task::switch_to_user_mode(uint32_t addr, uint32_t esp)
{
	switch_to_user_mode_called = true;
	asm volatile
	(
		"cli\n"
		"mov %0, %%ax\n"		// set user mode data selector
		"mov %%ax, %%ds\n"
		"mov %%ax, %%es\n"
		"mov %%ax, %%fs\n"
		"mov %%ax, %%gs\n"

		"pushl %0\n"
		"pushl %1\n"
		"pushf\n"

		"pop %%eax\n"
		"or $0x200, %%eax\n"	// set the IF flag
		"push %%eax\n"

		"pushl %2\n"			// set user mode code selector
		"pushl %3\n"
		"iret\n"
		: : "i"(USER_DATA_SELECTOR | 0x3), "g"(esp), "i"(USER_CODE_SELECTOR | 0x3), "g"(addr)
		: "eax"
	);
}

void move_stack()
{
	// we use static variables to avoid corruption
	static uint32_t i, old_esp, old_ebp, 
					offset, new_esp, new_ebp, tmp;

	for (i = 0; i < KERNEL_STACK_SIZE; i += 0x1000)
	{
		Page::current_page_dir->get_page(KERNEL_STACK_POS - 1 - i, true)->alloc(false, true);
		Page::invlpg(i);
	}

	asm volatile
	(
		"mov %%esp, %0\n"
		"mov %%ebp, %1\n"
		: "=g"(old_esp), "=g"(old_ebp)
	);

	offset = KERNEL_STACK_POS - initial_stack_pointer;
	new_esp = old_esp + offset;
	new_ebp = old_ebp + offset;

	memcpy((void*)new_esp, (void*)old_esp, initial_stack_pointer - old_esp);

	for (i = KERNEL_STACK_POS - 4; i > new_esp; i -= 4)
	{
		tmp = *(uint32_t*)i;

		// we assume all values between old_esp and initial_stack_pointer are a pointer ...
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

void set_errno(int errno)
{
	current_task->errno = errno;
}

volatile Task_t* get_next_task()
{
	return current_task->queue_next;
}

volatile Task_t* id2task(pid_t pid)
{
	Pair_id_task req;
	req.id = pid;
	Rbt_id_task::Node *ptr = rbt_id2task.find_ge(req);
	if (ptr)
		return ptr->get_key().task;
	return NULL;
}


void *Task_t::rbt_alloc()
{
	uint32_t addr = (uint32_t)Task_t::latest_task->rbt_node_mem;
	return (void*)(addr + (addr & 1));
}

void Task_t::rbt_free(void *)
{
}

