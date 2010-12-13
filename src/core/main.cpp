/*
 * $File: main.cpp
 * $Date: Fri Dec 10 11:57:32 2010 +0800
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
#include <lib/cxxsupport.h>
#include <lib/cstring.h>

static void init_timer();
static void timer_tick(Isr_registers_t reg);
static void isr_kbd(Isr_registers_t reg);

static int volatile last_key;

static void wait_key()
{
	last_key = 0;
	Scio::printf("press any key to continue...\n");
	while (!last_key);
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
	Task::init();
	Syscall::init();
	cxxsupport_init();

	init_timer();
	isr_register(ISR_GET_NUM_BY_IRQ(1), isr_kbd);

	asm volatile ("sti");

	Scio::printf("hello, world!\n");

	/*
	volatile int *x = (int*)0xF000000F;
	*x = 0;
	*/

	// wait_key();
	
	/*
	for (int l = 0; l < 3; l ++)
	{
		Scio::printf("\n\nLoop %d:\n", l);
		uint32_t ptr[3];
		for (int i = 0; i < 3; i ++)
		{
			Scio::printf("subloop %d\n", i);
			ptr[i] = (uint32_t)kmalloc(8 + i, 4);
			Scio::printf("\nptr[%d]=0x%x\n", i, ptr[i]);
			kheap_output_debug_msg();
			*(char*)ptr[i] = 'x';
			wait_key();
		}

		for (int i = 0; i < 3; i ++)
		{
			Scio::printf("subloop %d\n", i);
			kfree((void*)ptr[i]);
			Scio::printf("\nptr[%d] freed (orig addr=0x%x)\n", i, ptr[i]);
			kheap_output_debug_msg();
			wait_key();
		}
	}

	*/

	/*
	for (int i = 0; i < 3; i ++)
	{
		int volatile *x = (int*)kmalloc(sizeof(int));
		Scio::printf("addr=%p\n", x);
		wait_key();
		*x = 1;
		kfree((void*)x);
	}
	*/

	//kfree(0);

	/*
	int *ptr = new int[8192];
	memset(ptr, 0, sizeof(int) * 8192);
	kheap_output_debug_msg();
	delete []ptr;
	*/

	uint32_t uaddr = 0x54323456;
	Page::current_page_dir->get_page(uaddr, true)->alloc(true, true);
	uint32_t fbegin, fend;
	asm volatile
	(
		"jmp user_func_end\n"
		"user_func_begin:\n"
		"mov $1, %%eax\n"
		"mov %%ds, %%ebx\n"
		"iret\n"
		"int $0x80\n"
		"movl $1, 0x2\n"
		"1: jmp 1b\n"
		"user_func_end:\n"
		"movl $user_func_begin, %0\n"
		"movl $user_func_end, %1\n"
		: "=g"(fbegin), "=g"(fend)
	);
	memcpy((void*)uaddr, (void*)fbegin, fend - fbegin);

	Scio::printf("esp=0x%x\n", uaddr + 0x1000);

	Task::switch_to_user_mode(uaddr, uaddr + 0x1000);

	cxxsupport_finalize();
	panic("test");

	wait_key();
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
	//	Scio::printf("timer tick %d\n", tick);
	tick ++;
	// XXX: no task schedule
	return;
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

