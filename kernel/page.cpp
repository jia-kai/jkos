/*
 * $File: page.cpp
 * $Date: Mon Nov 29 19:27:48 2010 +0800
 *
 * x86 virtual memory management by paging
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

#include <page.h>
#include <scio.h>
#include <cstring.h>
#include <kheap.h>
#include <descriptor_table.h>

using namespace Page;

static Directory_t
	*kernel_page_dir,
	*current_page_dir;

// bitset of used frames
static Uint32_t *frames, nframes;

static void frame_set(Uint32_t addr);
static void frame_clear(Uint32_t addr);
static Uint32_t frame_unused(); // return an unused frame, or -1 if no such one

static void page_fault(Isr_registers_t reg); // page fault handler

void Table_entry_t::alloc(bool is_kernel, bool is_writable)
{
	if (!addr)
	{
		addr = frame_unused();
		if (addr == (Uint32_t)-1)
			PANIC("no free frame");
		present = 1;
		rw = is_writable ? 1 : 0;
		user = is_kernel ? 0 : 1;

		frame_set(addr);
	}
}

void Table_entry_t::free()
{
	if (addr)
	{
		frame_clear(addr);
		addr = 0;
		present = 0;
	}
}

Table_entry_t* Directory_t::get_page(Uint32_t addr, bool make)
{
	addr >>= 12;
	int tb_idx = addr >> 10,
		tb_offset = addr & 0x3FF;
	if (!tables[tb_idx])
	{
		if (!make)
			return NULL;

		Uint32_t phy;
		memset(
				tables[tb_idx] = (Table_t*)kmalloc_ap(sizeof(Table_t), &phy),
				0, sizeof(Table_t));

		entries[tb_idx].present = 1;
		entries[tb_idx].rw = 1;
		entries[tb_idx].user = 1;
		entries[tb_idx].addr = phy >> 12;
	}
	return &(tables[tb_idx]->pages[tb_offset]);
}

void Directory_t::enable()
{
	if (current_page_dir != this)
	{
		current_page_dir = this;
		asm volatile ("mov %0, %%cr3" : : "r"(&entries));
	}
}

void Page::init()
{
	// XXX: assume we have 16MB of RAM
	Uint32_t memsize = 0x1000000;

	nframes = memsize >> 12;
	frames = (Uint32_t*)kmalloc(nframes >> 3);
	memset(frames, 0, nframes >> 3);

	memset(
			kernel_page_dir = (Directory_t*)kmalloc_a(sizeof(Directory_t)),
			0, sizeof(Directory_t));

	for (Uint32_t i = 0; i < kheap_get_mem_size(); i += 0x1000)
		kernel_page_dir->get_page(i, true)->alloc(false, false);
	// kernel data are readable but not writable from userspace

	kernel_page_dir->enable();

	asm volatile
	(
		"mov %%cr0, %%eax\n"
		"or $0x80000000, %%eax\n"
		"mov %%eax, %%cr0"
		: : : "eax"
	);

	isr_register(14, page_fault);
}

void frame_set(Uint32_t addr)
{
	frames[addr >> 5] |= 1 << (addr & 31);
}

void frame_clear(Uint32_t addr)
{
	frames[addr >> 5] &= ~(1 << (addr & 31));
}

Uint32_t frame_unused()
{
	int it = nframes >> 5;
	for (int i = 0; i < it; i ++)
		if (frames[i] != 0xFFFFFFFF)
		{
			Uint32_t tmp = frames[i];
			int j = 0;
			while (tmp & 1)
				tmp >>= 1, j ++;
			return (i << 5) + j;
		}

	return -1;
}

void page_fault(Isr_registers_t reg)
{
	Uint32_t addr;
	asm volatile("mov %%cr2, %0" : "=r" (addr));

	Scio::printf("page fault: err_code=0x%x", reg.err_code);

	const char * BIT_MEAN[4][2] =
	{
		{"non-present", "protection violation"},
		{"read", "write"},
		{"supervisor", "user"},
		{NULL, "instruction fetch"}
	};
	bool first = true;
	for (int i = 0; i < 4; i ++)
	{
		int x = (reg.err_code >> i) & 1;
		if (BIT_MEAN[i][x])
		{
			if (first)
			{
				Scio::printf(" (");
				first = false;
			}
			else Scio::printf(", ");

			Scio::printf("%s", BIT_MEAN[i][x]);
		}
	}
	if (!first)
		Scio::printf(") ");

	Scio::printf(" at 0x%x\n", addr);

	PANIC("page fault");
}

