/*
 * $File: main.cpp
 * $Date: Tue Dec 21 11:40:54 2010 +0800
 *
 * This file contains the main routine of JKOS kernel
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

#include <multiboot.h>
#include <scio.h>
#include <descriptor_table.h>
#include <port.h>
#include <page.h>
#include <common.h>
#include <kheap.h>
#include <task.h>
#include <syscall.h>
#include <drv/ramdisk.h>
#include <lib/cxxsupport.h>
#include <lib/cstring.h>

static void init_timer();
static void timer_tick(Isr_registers_t reg);
static void isr_kbd(Isr_registers_t reg);

static int volatile last_key;

static void wait_key()
{
	Scio::printf("press any key to continue...\n");
	for (int volatile i = 0; i < 10000000; i ++);
	last_key = 0;
	while (!last_key);
}

void test_alloc()
{
#if 1
	for (int l = 0; l < 3; l ++)
	{
		Scio::printf("\n\nLoop %d:\n", l);
		void* ptr[3];
		for (int i = 0; i < 3; i ++)
		{
			Scio::printf("\nsubloop %d\n", i);
			ptr[i] = kmalloc(8 + i, 12);
			Scio::printf("arg=%d,%d  ptr[%d]=%p ",
					8 + i, 12, i, ptr[i]);
			*(volatile char*)ptr[i] = 'x';
			Scio::printf("phyaddr=%p\n", (void*)(Page::current_page_dir->get_physical_addr(ptr[i])));
			kheap_output_debug_msg();
			wait_key();
		}

		for (int i = 0; i < 3; i ++)
		{
			Scio::printf("\nsubloop %d\n", i);
			kfree((void*)ptr[i]);
			Scio::printf("ptr[%d] freed (orig addr=%p)\n", i, ptr[i]);
			kheap_output_debug_msg();
			wait_key();
		}
	}
#endif

	for (int i = 0; i < 4; i ++)
	{
		int *x = (int*)kmalloc(sizeof(int), i & 1 ? 12 : 0);
		Scio::printf("arg=%d,%d addr=%p ", sizeof(int),
				i & 1 ? 12 : 0, x);
		*(volatile int *)x = 1;
		Scio::printf("phyaddr=%p\n", (void*)(Page::current_page_dir->get_physical_addr(x)));
		kfree(x);
		wait_key();
	}

	int *ptr = new int[8192];
	Scio::printf("new int[8192]: addr=%p ", ptr);
	memset(ptr, 0, sizeof(int) * 8192);
	Scio::printf("phyaddr=%p\n", (void*)(Page::current_page_dir->get_physical_addr(ptr)));
	kheap_output_debug_msg();

	wait_key();
	delete []ptr;
}

void test_usermode()
{
	uint32_t uaddr = 0x54323000, uesp = uaddr + 0x500;
	Page::current_page_dir->get_page(uaddr, true)->alloc(true, true);
	uint32_t fbegin, fend;
#include <asm.h>
	asm volatile
	(
		"jmp user_func_end\n"
		"user_func_begin:\n"
		"mov %%esp, %%ebx\n"
		"mov $1, %%eax\n"
		"int $0x80\n"

#if 0
		"pushl %2\n"
		"pushl $kmain\n"
		"pushf\n"
		"pushl %3\n"
		"pushl %4\n"
		"iret\n"
#endif

		"mov $2, %%eax\n"
		"int $0x80\n"
		"mov %%eax, %%ebx\n"
		"mov $1, %%eax\n"
		"mov $10, %%ecx\n"
		"loop:\n"
		"int $0x80\n"
		"mov $1000000, %%edx\n"
		"1:\n"
		"dec %%edx\n"
		"cmpl $0, %%edx\n"
		"jne 1b\n"
		"dec %%ecx\n"
		"cmpl $0, %%ecx\n"
		"jne loop\n"

		"addl $100, %%ebx\n"
		"int $0x80\n"

		"1: jmp 1b\n"
		"user_func_end:\n"
		"movl $user_func_begin, %0\n"
		"movl $user_func_end, %1\n"
		: "=g"(fbegin), "=g"(fend)
		: "i"(KERNEL_DATA_SELECTOR), "i"(KERNEL_CODE_SELECTOR), "i"(0)
	);
	memcpy((void*)uaddr, (void*)fbegin, fend - fbegin);

	Scio::printf("esp should be 0x%x\n", uesp);

	Task::switch_to_user_mode(uaddr, uesp);

}

void test_fork(Multiboot_info_t *mbd)
{
	Task::pid_t fork_ret = Task::fork();
	Task::pid_t pid = Task::getpid();

	for (int i = 0; i < 10; i ++)
	{
		Scio::printf("fork returned: %d   pid: %d\n", fork_ret, pid);
		for (int volatile j = 0; j < 1000000; j ++);
	}

	if (mbd->mods_count)
	{
		uint32_t start = *(uint32_t*)mbd->mods_addr,
				 end = *(uint32_t*)(mbd->mods_addr + 4);

		MSG_INFO("ramdisk found: start=0x%x end=0x%x", start, end);

		Fs::Node_file *file = ramdisk_get_file_node(start, end);
		char buf[256];
		memset(buf, 0, sizeof(buf));
		for (int i = 0; i < 2; i ++)
		{
			file->read(buf, 256);
			Scio::printf("(pid %d) ramdisk: %s\n", pid, buf);
			buf[0] = (char)('0' + pid);
			file->write(buf, 256);
			for (int volatile j = 0; j < 1000000; j ++);
		}
	}

	Scio::printf("(pid %d) done\n", pid);
	for (; ;);
}

extern "C" void kmain(Multiboot_info_t *mbd, uint32_t magic)
{
	init_descriptor_tables();
	Scio::init();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		Scio::printf("invalid magic: 0x%x\n", magic);
		return;
	}

	Page::init(mbd);
	cxxsupport_init();
	Task::init();
	Syscall::init();

	init_timer();
	isr_register(ISR_GET_NUM_BY_IRQ(1), isr_kbd);

	asm volatile ("sti");

	Scio::printf("hello, world!\n");

	/*
	volatile int *x = NULL;
	*x = 0;
	*/

	// test_alloc();
	test_fork(mbd);

	cxxsupport_finalize();
}

void init_timer()
{
	isr_register(ISR_GET_NUM_BY_IRQ(0), timer_tick);

	using namespace Port;
	uint32_t divisor = CLOCK_TICK_RATE / KERNEL_HZ;
	outb(0x43, 0b00110110);
	wait();
	outb(0x40, (uint8_t)(divisor & 0xFF));
	wait();
	outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void timer_tick(Isr_registers_t reg)
{
	static int tick;
	//if (tick % 100 == 0)
	// Scio::printf("timer tick %d\n", tick);
	tick ++;
	isr_eoi(reg.int_no);
	Task::schedule();
}

void isr_kbd(Isr_registers_t reg)
{
	last_key = 0;
	uint8_t code = Port::inb(0x60);

	// Scio::printf("keyboard scancode: 0x%x\n", code);

	last_key = code;

	isr_eoi(reg.int_no);
}

