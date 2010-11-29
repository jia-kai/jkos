/*
 * $File: main.cpp
 * $Date: Mon Nov 29 19:34:44 2010 +0800
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

static void init_timer();
static void timer_tick(Isr_registers_t reg);

extern "C" void kmain(Multiboot_info_t* , unsigned int magic)
{
	init_descriptor_tables();
	Scio::init();
	Page::init();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		Scio::printf("invalid magic: 0x%x\n", magic);
		return;
	}

	init_timer();

	asm volatile ("sti");

	asm volatile ("int $30");

	asm volatile ("int $31");
	asm volatile ("int $30");

	Scio::printf("hello, world!\n");

	volatile int *ptr = (int*)0xF000000F;
	*ptr = 0;

	for (; ;);
}

void init_timer()
{
	isr_register(ISR_GET_NUM_BY_IRQ(0), timer_tick);

	using namespace Port;
	int divisor = CLOCK_TICK_RATE / KERNEL_HZ;
	outb(0x43, 0b00110110);
	wait();
	outb(0x40, divisor & 0xFF);
	wait();
	outb(0x40, (divisor >> 8) & 0xFF);
}

void timer_tick(Isr_registers_t reg)
{
	static int tick;
	if (tick % 100 == 0)
		Scio::printf("timer tick %d\n", tick);
	tick ++;
	isr_eoi(reg.int_no);
}

