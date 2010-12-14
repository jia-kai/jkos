/*
 * $File: kheap.cpp
 * $Date: Tue Dec 14 20:23:34 2010 +0800
 *
 * manipulate kernel heap (virtual memory)
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

#include <lib/rbtree.h>
#include <kheap.h>
#include <common.h>
#include <page.h>
#include <scio.h>

// defined in the linker script
extern "C" uint32_t start_ctors, kheap_start_ctors, kernel_img_end;

// used for dynamic memory allocating
static uint32_t kheap_begin, kheap_end;

// used for static memory allocating (before page initialization completed)
static uint32_t kheap_static_end = (uint32_t)&kernel_img_end + 4;

// I think 256MB kernel heap is enough
static const uint32_t
	KERNEL_HEAP_END = 0xFFFFF000u,
	KERNEL_HEAP_SIZE = 256 * 1024 * 1024;

// used for memory allocating before calling kheap_init
static void* kmalloc_pre_init(uint32_t size, int palign);


struct Block_t
{
	// unallocated memory
	uint32_t start, size;
	inline bool operator < (const Block_t &n) const
	{ return size < n.size || (size == n.size && start < n.start); }
	inline bool operator <= (const Block_t &n) const
	{ return size < n.size || (size == n.size && start <= n.start); }
	inline bool operator == (const Block_t &n) const
	{ return size == n.size && start == n.start; }
};

struct Hole_t
{
	// allocated memory chunk
	uint32_t start, size;
	inline bool operator < (const Hole_t &n) const
	{ return start < n.start; }
	inline bool operator <= (const Hole_t &n) const
	{ return start <= n.start; }
};

namespace Tree_mm
{
	// memory manager for rbt
	static const int
		STATIC_SIZE = 1024,
		TREE_NODE_SIZE = sizeof(Rbt<Block_t>::Node),
		STATIC_MEM_SIZE = STATIC_SIZE * TREE_NODE_SIZE;
	static uint8_t static_mem[STATIC_MEM_SIZE] __attribute__((aligned(2)));
	static void* freed[STATIC_SIZE];
	static int nstatic_mem, nfreed;

	static void* alloc();
	static void free(void *ptr);
}

static Rbt<Block_t> tree_block(Tree_mm::alloc, Tree_mm::free);
static Rbt<Hole_t> tree_hole(Tree_mm::alloc, Tree_mm::free);

static inline uint32_t get_aligned(uint32_t addr, int palign)
{ if (!addr) return 0; return (((addr - 1) >> palign) + 1) << palign; }

void* kmalloc(uint32_t size, int palign)
{
	if (!kheap_begin) // kheap_finish_init has not been called
		return kmalloc_pre_init(size, palign);

	Block_t req;
	req.size = size;
	req.start = 0;
	while (1)
	{
		Rbt<Block_t>::Node *ptr = tree_block.find_ge(req);
		if (!ptr)
			panic("can not allocate virtual memory chunk");
		Block_t got = ptr->get_key();
		uint32_t as = get_aligned(got.start, palign); // aligned start address
		if (as - got.start + size > got.size)
		{
			req = got;
			req.start ++;
			continue;
		}

		tree_block.erase(ptr);
		Block_t nblock;
		nblock.start = as + size;
		nblock.size = got.start + got.size - nblock.start;
		if (nblock.size)
			tree_block.insert(nblock);

		Hole_t hole;
		hole.start = got.start;
		hole.size = as - got.start + size;
		tree_hole.insert(hole);

		return (void*)as;
	}
}

void kfree(void *addr)
{
	Hole_t req;
	req.start = (uint32_t)addr;
	Rbt<Hole_t>::Node *ptr = tree_hole.find_le(req);

	if (!ptr)
		panic("attempt to free a non-existing memory chunk");

	Hole_t got = ptr->get_key();
	tree_hole.erase(ptr);

	Block_t nblock; // new block to be inserted
	nblock.start = got.start;
	nblock.size = got.size;

	Hole_t tmp; // the hole adjacent to newly freed block

	uint32_t hole_left_end, hole_right_start;

	ptr = tree_hole.find_le(got);
	if (ptr) // try to merge with the left block
		tmp = ptr->get_key();
	else
		tmp.start = kheap_begin, tmp.size = 0;
	hole_left_end = tmp.start + tmp.size;
	if (hole_left_end != nblock.start)
	{
		Block_t breq;
		breq.start = tmp.start + tmp.size;
		breq.size = nblock.start - breq.start;
		Rbt<Block_t>::Node *p = tree_block.find_ge(breq);
		kassert(p != NULL && p->get_key() == breq);

		tree_hole.erase(ptr);
		tree_block.erase(p);

		nblock.start = breq.start;
		nblock.size += breq.size;
	}

	ptr = tree_hole.find_ge(got);
	if (ptr) // try to merge with the left block
		tmp = ptr->get_key();
	else
		tmp.start = kheap_end, tmp.size = 0;
	hole_right_start = tmp.start;
	if (nblock.start + nblock.size != hole_right_start)
	{
		Block_t breq;
		breq.start = nblock.start + nblock.size;
		breq.size = tmp.start - breq.start;
		Rbt<Block_t>::Node *p = tree_block.find_ge(breq);
		kassert(p != NULL && p->get_key() == breq);

		tree_hole.erase(ptr);
		tree_block.erase(p);

		nblock.size += breq.size;
	}

	tree_block.insert(nblock);

	// free used pages
	uint32_t
		start = max(hole_left_end, got.start & 0xFFFFF000),
		end = min(hole_right_start, get_aligned(got.start + got.size, 12));
	for (uint32_t i = get_aligned(start, 12); i + 0x1000 <= end; i += 0x1000)
	{
		Page::Table_entry_t *page = Page::current_page_dir->get_page(i, false);
		if (page)
		{
			page->free();
			Page::invlpg(i);
		}
	}
}

uint32_t kheap_get_size_pre_init()
{
	return kheap_static_end;
}

void kheap_preserve_mem(uint32_t addr)
{
	if (addr > kheap_static_end)
		kheap_static_end = addr;
}

void* kmalloc_pre_init(uint32_t size, int palign)
{
	kheap_static_end = get_aligned(kheap_static_end, palign);
	uint32_t ret = kheap_static_end;
	kheap_static_end += size;
	return (void*)ret;
}

void kheap_init()
{
	for(uint32_t *ptr = &kheap_start_ctors; ptr < &start_ctors; ptr ++)
		((void (*)(void))*ptr)();

	Block_t b;
	b.start = KERNEL_HEAP_END - KERNEL_HEAP_SIZE;
	b.size = KERNEL_HEAP_SIZE;
	tree_block.insert(b);

	for (uint32_t i = b.start; i < KERNEL_HEAP_END; i += 0x1000)
		Page::current_page_dir->get_page(i, true);
}

void kheap_finish_init()
{
	kheap_begin = KERNEL_HEAP_END - KERNEL_HEAP_SIZE;
	kheap_end = KERNEL_HEAP_END;
	MSG_INFO("kernel heap address range: %p %p", (void*)kheap_begin, (void*)(kheap_end - 1));
}

void* Tree_mm::alloc()
{
	if (nfreed)
		return freed[-- nfreed];

	if (nstatic_mem + TREE_NODE_SIZE >= STATIC_MEM_SIZE)
		panic("run out of kernel heap");

	void *ret = static_mem + nstatic_mem;
	nstatic_mem += TREE_NODE_SIZE;
	return ret;
}

void Tree_mm::free(void *ptr)
{
	freed[nfreed ++] = ptr;
}

#ifdef DEBUG
static void walk_block(const Block_t &b)
{
	Scio::printf("unallocated block: start=0x%x size=0x%x\n", b.start, b.size);
}

static void walk_hole(const Hole_t &h)
{
	Scio::printf("allocated: start=0x%x size=0x%x\n", h.start, h.size);
}

void kheap_output_debug_msg()
{
	using namespace Scio;

	push_color(LIGHT_GREEN, BLACK);
	puts("start kernel heap debug output\n");

	printf("number of nodes ever allocated: %d\n", Tree_mm::nstatic_mem / Tree_mm::TREE_NODE_SIZE);
	printf("number of currently ununsed nodes: %d\n", Tree_mm::nfreed);

	tree_block.walk(walk_block);
	tree_hole.walk(walk_hole);

	puts("end kernel heap debug output\n");
	pop_color();
}
#endif // DEBUG

