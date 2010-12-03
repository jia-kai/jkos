/*
 * $File: main.cpp
 * $Date: Fri Dec 03 21:10:12 2010 +0800
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
#include <lib/cxxsupport.h>
#include <lib/cstring.h>

static void init_timer();
static void timer_tick(Isr_registers_t reg);
static void isr_kbd(Isr_registers_t reg);

static int last_key;

static void wait_key()
{
	last_key = 0;
	Scio::printf("press any key to continue...\n");
	while (!last_key);
}

extern "C" void kmain(Multiboot_info_t *mbd, unsigned int magic)
{
	init_descriptor_tables();
	Scio::init();
	Page::init(mbd);
	cxxsupport_init();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		Scio::printf("invalid magic: 0x%x\n", magic);
		return;
	}

	init_timer();
	isr_register(ISR_GET_NUM_BY_IRQ(1), isr_kbd);

	asm volatile ("sti");

	asm volatile ("int $30");

	asm volatile ("int $31");
	asm volatile ("int $30");

	Scio::printf("hello, world!\n");

	/*
	volatile int *x = (int*)0xF000000F;
	*x = 0;
	*/

	wait_key();
	
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

	int *ptr = new int[8192];
	memset(ptr, 0, sizeof(int) * 8192);
	kheap_output_debug_msg();
	delete []ptr;
	
	cxxsupport_finalize();
	panic("test");
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
	isr_eoi(reg.int_no);
}

void isr_kbd(Isr_registers_t reg)
{
	uint8_t code = Port::inb(0x60);

	// Scio::printf("keyboard scancode: 0x%x\n", code);

	last_key = code;

	isr_eoi(reg.int_no);
}

