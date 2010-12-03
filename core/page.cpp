/*
 * $File: page.cpp
 * $Date: Fri Dec 03 15:40:52 2010 +0800
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
#include <lib/cstring.h>
#include <descriptor_table.h>
#include <kheap.h>

#pragma GCC diagnostic ignored "-Wconversion"

// defined in the linker script
extern "C" Uint32_t kernel_img_end;

// used for initializing
static Uint32_t kheap_end = ((Uint32_t)&kernel_img_end) + 1;


using namespace Page;

static Directory_t *current_page_dir;
Directory_t *Page::kernel_page_dir;

// bitset of used frames
static Uint32_t *frames, nframes;

static void frame_set(Uint32_t addr);
static void frame_clear(Uint32_t addr);
static Uint32_t frame_unused(); // return an unused frame, or -1 if no such one

// allocate memory during initialization
static void* init_malloc(Uint32_t size, int palign = 0);

static void page_fault(Isr_registers_t reg); // page fault handler

void Table_entry_t::alloc(bool is_kernel, bool is_writable)
{
	if (!addr)
	{
		addr = frame_unused();
		if (addr == (Uint32_t)-1)
			panic("no free frame");
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

Table_entry_t* Directory_t::get_page(Uint32_t addr, bool make, bool rw, bool user)
{
	addr >>= 12;
	int tb_idx = addr >> 10,
		tb_offset = addr & 0x3FF;
	if (!tables[tb_idx])
	{
		if (!make)
			return NULL;

		memset(
				tables[tb_idx] = static_cast<Table_t*>(kmalloc(sizeof(Table_t), 12)),
				0, sizeof(Table_t));

		entries[tb_idx].present = 1;
		entries[tb_idx].rw = rw ? 1 : 0;
		entries[tb_idx].user = user ? 1 : 0;
		entries[tb_idx].addr = get_physical_addr((Uint32_t)tables[tb_idx], true, true, true) >> 12;
	}
	return &(tables[tb_idx]->pages[tb_offset]);
}

void Directory_t::enable()
{
	if (current_page_dir != this)
	{
		current_page_dir = this;
		asm volatile ("mov %0, %%cr3" : : "r"(phyaddr));
	}
}

Uint32_t Directory_t::get_physical_addr(Uint32_t addr, bool alloc, bool is_kernel, bool is_writable)
{
	Table_entry_t *page = get_page(addr, false);
	if (page == NULL)
		return (Uint32_t)-1;
	if (!page->addr)
	{
		if (!alloc)
			return (Uint32_t)-1;
		page->alloc(is_kernel, is_writable);
	}
	return (page->addr << 12) | (addr & 0xFFF);
}

void Page::init()
{
	// XXX: assume we have 16MB of RAM
	Uint32_t memsize = 0x1000000;

	nframes = memsize >> 12;
	frames = static_cast<Uint32_t*>(init_malloc(nframes >> 3));
	memset(frames, 0, nframes >> 3);

	memset(
			kernel_page_dir = static_cast<Directory_t*>(init_malloc(sizeof(Directory_t), 12)),
			0, sizeof(Directory_t));

	for (Uint32_t i = 0; i < kheap_end; i += 0x1000)
	{
		int addr = i >> 12,
			tb_idx = addr >> 10,
			tb_offset = addr & 0x3FF;
		if (!kernel_page_dir->tables[tb_idx])
		{
			memset(
					kernel_page_dir->tables[tb_idx] = static_cast<Table_t*>(
						init_malloc(sizeof(Table_t), 12)),
					0, sizeof(Table_t));

			kernel_page_dir->entries[tb_idx].present = 1;
			kernel_page_dir->entries[tb_idx].rw = 1;
			kernel_page_dir->entries[tb_idx].user = 1;
			kernel_page_dir->entries[tb_idx].addr = ((Uint32_t)kernel_page_dir->tables[tb_idx]) >> 12;

			kernel_page_dir->phyaddr = (Uint32_t)(kernel_page_dir->entries);
		}
		Table_entry_t &page = kernel_page_dir->tables[tb_idx]->pages[tb_offset];
		page.present = 1;
		page.rw = 0;
		page.user = 1;
		page.addr = addr;
		frame_set(addr);
	}

	kernel_page_dir->enable();

	asm volatile
	(
		"mov %%cr0, %%eax\n"
		"or $0x80000000, %%eax\n"
		"mov %%eax, %%cr0"
		: : : "eax"
	);

	isr_register(14, page_fault);

	kheap_init(kheap_end, 0xFFFFFFFFu - kheap_end);

	MSG_INFO("page initialization completed. kernel static memory usage: %d kb",
			kheap_end >> 10);
}

void frame_set(Uint32_t addr)
{
	frames[addr >> 5] |= 1 << (addr & 31);
}

void frame_clear(Uint32_t addr)
{
	MSG_DEBUG("frame 0x%x freed", addr);
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
			MSG_DEBUG("new frame allocated: 0x%x", (i << 5) + j);
			return (i << 5) + j;
		}

	return -1;
}

void* init_malloc(Uint32_t size, int palign)
{
	kheap_end = (((kheap_end - 1) >> palign) + 1) << palign;
	Uint32_t ret = kheap_end;
	kheap_end += size;
	return (void*)ret;
}

void page_fault(Isr_registers_t reg)
{
	Uint32_t addr;
	asm volatile("mov %%cr2, %0" : "=r" (addr));

	Table_entry_t *page = current_page_dir->get_page(addr, false);
	if (page && !(reg.err_code & 1))
	{
		// XXX: all allocated frames are kernel used and writable
		page->alloc(true, true);
		return;
	}

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

	panic("page fault");
}

