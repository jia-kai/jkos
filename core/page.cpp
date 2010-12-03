/*
 * $File: page.cpp
 * $Date: Fri Dec 03 21:39:57 2010 +0800
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

#include <lib/cstring.h>
#include <page.h>
#include <multiboot.h>
#include <scio.h>
#include <descriptor_table.h>
#include <kheap.h>

#pragma GCC diagnostic ignored "-Wconversion"

// defined in the linker script
extern "C" uint32_t kernel_img_end;

// used for initializing
static uint32_t kheap_end = ((uint32_t)&kernel_img_end) + 1;


using namespace Page;

static Directory_t *current_page_dir;
Directory_t *Page::kernel_page_dir;

// stack of usable frames
static uint32_t *frames, nframes;

static void init_frames(Multiboot_info_t *mbd);

// allocate memory during initialization
static void* init_malloc(uint32_t size, int palign = 0);

static void page_fault(Isr_registers_t reg); // page fault handler

void Table_entry_t::alloc(bool is_kernel, bool is_writable)
{
	if (!addr)
	{
		if (!nframes)
			panic("no free frame");
		addr = frames[-- nframes];
		MSG_DEBUG("frame 0x%x allocated", addr);
		present = 1;
		rw = is_writable ? 1 : 0;
		user = is_kernel ? 0 : 1;
	}
}

void Table_entry_t::free()
{
	if (addr)
	{
		frames[nframes ++] = addr;
		MSG_DEBUG("frame 0x%x freed", addr);
		addr = 0;
		present = 0;
	}
}

Table_entry_t* Directory_t::get_page(uint32_t addr, bool make, bool rw, bool user)
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
		entries[tb_idx].addr = get_physical_addr((uint32_t)tables[tb_idx], true, true, true) >> 12;
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

uint32_t Directory_t::get_physical_addr(uint32_t addr, bool alloc, bool is_kernel, bool is_writable)
{
	Table_entry_t *page = get_page(addr, false);
	if (page == NULL)
		return (uint32_t)-1;
	if (!page->addr)
	{
		if (!alloc)
			return (uint32_t)-1;
		page->alloc(is_kernel, is_writable);
	}
	return (page->addr << 12) | (addr & 0xFFF);
}

void Page::init(void *ptr_mbd)
{
	Multiboot_info_t *mbd = static_cast<Multiboot_info_t*>(ptr_mbd);
	kassert(mbd->flags & MULTIBOOT_INFO_MEM_MAP);

	memset(
			kernel_page_dir = static_cast<Directory_t*>(init_malloc(sizeof(Directory_t), 12)),
			0, sizeof(Directory_t));

	init_frames(mbd);
	for (uint32_t i = 0; i < kheap_end; i += 0x1000)
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
			kernel_page_dir->entries[tb_idx].addr = ((uint32_t)kernel_page_dir->tables[tb_idx]) >> 12;

		}
		Table_entry_t &page = kernel_page_dir->tables[tb_idx]->pages[tb_offset];
		page.present = 1;
		page.rw = 0;
		page.user = 1;
		page.addr = addr;
	}
	while (frames[nframes - 1] <= (kheap_end >> 12))
		nframes --;
	kernel_page_dir->phyaddr = (uint32_t)(kernel_page_dir->entries);

	kernel_page_dir->enable();

	asm volatile
	(
		"mov %%cr0, %%eax\n"
		"or $0x80000000, %%eax\n"
		"mov %%eax, %%cr0"
		: : : "eax"
	);

	isr_register(14, page_fault);

	kheap_init(kheap_end, 0xFFFFEFFFu - kheap_end);

	MSG_INFO("page initialization completed.\n kernel static memory usage: %d kb\n usable 4-kb frames: %d",
			kheap_end >> 10, nframes);
}

void* init_malloc(uint32_t size, int palign)
{
	kheap_end = (((kheap_end - 1) >> palign) + 1) << palign;
	uint32_t ret = kheap_end;
	kheap_end += size;
	return (void*)ret;
}

void page_fault(Isr_registers_t reg)
{
	uint32_t addr;
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

void init_frames(Multiboot_info_t *mbd)
{
	uint32_t memsize = 0;
	uint32_t mmap_addr = mbd->mmap_addr;
	while (mmap_addr < mbd->mmap_addr + mbd->mmap_length)
	{
		Multiboot_mmap_entry_t *mmap = (Multiboot_mmap_entry_t*)mmap_addr;

		MSG_INFO("memory map: %s: start=0x%x len=%d kb",
				mmap->type == MULTIBOOT_MEMORY_AVAILABLE ?  "available" : "reserved",
				(uint32_t)mmap->addr, (uint32_t)mmap->len >> 10);

		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
		{
			uint64_t end = mmap->addr + mmap->len;

			if (mmap->addr >= 0xFFFFFFFFull || mmap->len >= 0xFFFFFFFFull ||
					end > 0xFFFFFFFFull)
				panic("physical memory too large");


			if (end > memsize)
				memsize = (uint32_t)end;
		}

		mmap_addr += mmap->size + 4;
	}

	frames = static_cast<uint32_t*>(init_malloc((memsize >> 12) << 2));
	nframes = 0;

	mmap_addr = mbd->mmap_addr;
	while (mmap_addr < mbd->mmap_addr + mbd->mmap_length)
	{
		Multiboot_mmap_entry_t *mmap = (Multiboot_mmap_entry_t*)mmap_addr;
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
		{
			uint32_t start = (uint32_t)mmap->addr;
			if (start)
				start = ((start - 1) >> 12) + 1;
			uint32_t end = (uint32_t)(mmap->addr + mmap->len);
			end >>= 12;

			for (uint32_t i = end; (-- i) >= start; )
				if (i > (kheap_end >> 12))
					frames[nframes ++] = i;
				else break;
		}
		mmap_addr += mmap->size + 4;
	}
}

