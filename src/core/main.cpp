/*
 * $File: main.cpp
 * $Date: Wed Dec 29 17:39:54 2010 +0800
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
#include <klog.h>
#include <descriptor_table.h>
#include <port.h>
#include <page.h>
#include <common.h>
#include <kheap.h>
#include <task.h>
#include <elf.h>
#include <drv/ramdisk.h>
#include <lib/cxxsupport.h>
#include <lib/cstring.h>

static void init_timer();
static void timer_tick(Isr_registers_t reg);
static void isr_kbd(Isr_registers_t reg);

static int volatile last_key;

static void wait_key()
{
	Klog::printf("press any key to continue...\n");
	for (int volatile i = 0; i < 10000000; i ++);
	last_key = 0;
	while (!last_key);
}

void test_alloc()
{
#if 1
	for (int l = 0; l < 3; l ++)
	{
		Klog::printf("\n\nLoop %d:\n", l);
		void* ptr[3];
		for (int i = 0; i < 3; i ++)
		{
			Klog::printf("\nsubloop %d\n", i);
			ptr[i] = kmalloc(8 + i, 12 - i);
			Klog::printf("arg=%d,%d  ptr[%d]=%p ",
					8 + i, 12 - i, i, ptr[i]);
			*(volatile char*)ptr[i] = 'x';
			Klog::printf("phyaddr=%p\n", (void*)(Page::current_page_dir->get_physical_addr(ptr[i])));
			kheap_output_debug_msg();
			wait_key();
		}

		for (int i = 0; i < 3; i ++)
		{
			Klog::printf("\nsubloop %d\n", i);
			kfree((void*)ptr[i]);
			Klog::printf("ptr[%d] freed (orig addr=%p)\n", i, ptr[i]);
			kheap_output_debug_msg();
			wait_key();
		}
	}
#endif

	for (int i = 0; i < 4; i ++)
	{
		int *x = (int*)kmalloc(sizeof(int), i & 1 ? 12 : 0);
		Klog::printf("arg=%d,%d addr=%p ", sizeof(int),
				i & 1 ? 12 : 0, x);
		*(volatile int *)x = 1;
		Klog::printf("phyaddr=%p\n", (void*)(Page::current_page_dir->get_physical_addr(x)));
		kfree(x);
		wait_key();
	}

	int *ptr = new int[8192];
	Klog::printf("new int[8192]: addr=%p ", ptr);
	memset(ptr, 0, sizeof(int) * 8192);
	Klog::printf("phyaddr=%p\n", (void*)(Page::current_page_dir->get_physical_addr(ptr)));
	kheap_output_debug_msg();

	wait_key();
	delete []ptr;
}

void test_fork(Multiboot_info_t *mbd)
{
	pid_t fork_ret = Task::fork();
	Klog::printf("fork returned %d\n", fork_ret);
	pid_t pid = Task::getpid();

	for (int i = 0; i < 5; i ++)
	{
		Klog::printf("fork returned: %d   pid: %d\n", fork_ret, pid);
		for (int volatile j = 0; j < 1000000; j ++);
	}

	if (mbd->mods_count)
	{
		uint32_t start = *(uint32_t*)mbd->mods_addr,
				 end = *(uint32_t*)(mbd->mods_addr + 4);

		Klog::log(Klog::INFO, "ramdisk found: start=0x%x end=0x%x", start, end);

		Fs::Node_file *file = ramdisk_get_file_node(start, end);
		char buf[256];
		memset(buf, 0, sizeof(buf));
		for (int i = 0; i < 2; i ++)
		{
			file->read(buf, 256);
			Klog::printf("(pid %d) ramdisk: %s\n", pid, buf);
			buf[0] = (char)('0' + pid);
			file->write(buf, 256);
			for (int volatile j = 0; j < 1000000; j ++);
		}
	}

	Klog::printf("(pid %d) done\n", pid);
	for (; ;);
}

void test_sleep()
{
	Task::fork();
	pid_t pid = Task::getpid();
	if (pid == 0)
		Task::sleep(0, Sigset()); // sleep self

	for (int i = 0; ; i ++)
	{
		if (i == 10)
		{
			if (pid == 1)
				Task::wakeup(0);
			else if (pid == 0)
				Task::sleep(1, Sigset()); // sleep other
		}
		if (i == 20 && pid == 0)
			Task::wakeup(1);
		Klog::printf("pid %d: %d\n", pid, i);
		for (int volatile j = 0; j  < 10000000; j ++);
	}
}

void test_elf(Multiboot_info_t *mbd)
{
	kassert(mbd->mods_count);
	uint32_t start = *(uint32_t*)mbd->mods_addr,
			 end = *(uint32_t*)(mbd->mods_addr + 4);
	uint32_t entry = load_elf(ramdisk_get_file_node(start, end)),
			 stack = 0xC0000000;

	Page::current_page_dir->get_page(stack, true)->alloc(true, true);

	Task::switch_to_user_mode(entry, stack + 0x1000 - 4);
}

void test_lazy_alloc()
{
	for (int i = 0; i <= 0x5000; i += 0x1000)
	{
		Page::Table_entry_t *pg = Page::current_page_dir->get_page(0x60000000 + i, true);
		pg->alloc(false, true);
		pg->fill_uint32(12345);
	}
	for (int i = 0; i <= 0x5000; i += 0x1000)
		Page::current_page_dir->get_page(0x60000000 + i)->free();
	Klog::printf("lazy alloc (not filling)\n");
	uint32_t addr = 0x67890000;
	Page::current_page_dir->get_page(addr, true)->lazy_alloc(false, true, false);
	Klog::printf("checking...\n");
	for (uint32_t i = 0; i < 0x1000; i += 4)
	{
		uint32_t *ptr = (uint32_t*)(i + addr);
		if (*ptr)
		{
			Klog::printf("%p: %d\n", ptr, *ptr);
			break;
		}
	}
	addr += 0x1000;
	Klog::printf("lazy alloc (filling)\n");
	Page::current_page_dir->get_page(addr, true)->lazy_alloc(false, true, true);
	Klog::printf("checking...\n");
	for (uint32_t i = 0; i < 0x1000; i += 4)
	{
		uint32_t *ptr = (uint32_t*)(i + addr);
		if (*ptr)
			panic("%p: %d: not zero!\n", ptr, *ptr);
	}
	panic("done");
}

extern "C" void kmain(Multiboot_info_t *mbd, uint32_t magic)
{
	init_descriptor_tables();
	Klog::init();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		Klog::printf("invalid magic: 0x%x\n", magic);
		return;
	}

	Page::init(mbd);
	cxxsupport_init();
	Task::init();

	init_timer();
	isr_register(ISR_GET_NUM_BY_IRQ(1), isr_kbd);

	asm volatile ("sti");

	Klog::printf("hello, world!\n");

	/*
	volatile int *x = NULL;
	*x = 0;
	*/

	// test_alloc();
	// test_fork(mbd);
	// test_sleep();
	// test_lazy_alloc();
	test_elf(mbd);

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
	// if (tick % 100 == 0)
	//	Klog::printf("timer tick %d\n", tick);
	tick ++;
	isr_eoi(reg.int_no);
	Task::schedule();
}

void isr_kbd(Isr_registers_t reg)
{
	last_key = 0;
	uint8_t code = Port::inb(0x60);

	// Klog::printf("keyboard scancode: 0x%x\n", code);

	last_key = code;

	isr_eoi(reg.int_no);
}

