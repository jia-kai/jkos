/*
 * $File: kheap.cpp
 * $Date: Wed Dec 29 20:17:41 2010 +0800
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

#include <kheap.h>
#include <common.h>
#include <page.h>
#include <klog.h>
#include <lib/rbtree.h>
#include <lib/cstring.h>

// defined in the linker script
extern "C" uint32_t start_ctors, kheap_start_ctors, kernel_img_end;

static bool kheap_finish_init_called = false;

// used for static memory allocating (before page initialization completed)
static uint32_t kheap_static_end = (uint32_t)&kernel_img_end + 4;

// used for memory allocating before calling kheap_init
static void* kmalloc_pre_init(uint32_t size, int palign);

uint32_t USER_MEM_LOW, USER_MEM_HIGH;

struct Block_t
{
	uint32_t start, size;
};

struct Block_size_t: public Block_t
{
	// unallocated memory block, sort by size
	inline bool operator < (const Block_size_t &n) const
	{ return size < n.size || (size == n.size && start < n.start); }
	
	Block_size_t(const Block_t &b) : Block_t::Block_t(b) {}
	Block_size_t() {}
};

struct Block_start_t: public Block_t
{
	// unallocated memory block, sort by start
	inline bool operator < (const Block_start_t &n) const
	{ return start < n.start || (start == n.start && size < n.size);}
	inline bool operator <= (const Block_start_t &n) const
	{ return start < n.start || (start == n.start && size <= n.size);}

	Block_start_t(const Block_t &b) : Block_t::Block_t(b) {}
	Block_start_t() {}
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

static Rbt<Block_size_t> tree_size(Tree_mm::alloc, Tree_mm::free);
static Rbt<Block_start_t> tree_start(Tree_mm::alloc, Tree_mm::free);

void* kmalloc(uint32_t size, int palign)
{
	if (!kheap_finish_init_called)
		return kmalloc_pre_init(size, palign);

	Block_t req;
	req.size = size + sizeof(Block_t);
	req.start = 0;
	while (1)
	{
		Rbt<Block_size_t>::Node *ptr = tree_size.find_ge(req);
		if (!ptr)
			panic("kernel runs out of memory");

		Block_t got(ptr->get_key());
		uint32_t start = get_aligned(got.start + sizeof(Block_t), palign);
		if (start + size > got.start + got.size)
		{
			req = got;
			req.start ++;
			continue;
		}

		tree_size.erase(ptr);
		tree_start.erase(tree_start.find_ge(got));

		if (start + size + sizeof(Block_t) * 2 < got.start + got.size)
		{
			Block_t nblk;
			nblk.start = start + size;
			nblk.size = got.start + got.size - nblk.start;

			got.size -= nblk.size;

			tree_size.insert(nblk);
			tree_start.insert(nblk);
		}

		if (start - got.start > sizeof(Block_t) * 3)
		{
			Block_t nblk;
			nblk.start = got.start;
			nblk.size = start - got.start - sizeof(Block_t);

			got.size -= nblk.size;
			got.start += nblk.size;

			tree_size.insert(nblk);
			tree_start.insert(nblk);
		}

		Page::current_page_dir->lazy_alloc_interval(
				(start - sizeof(Block_t)) & 0xFFFFF000, get_aligned(start + size, 12),
				false, true, false);

		memcpy((void*)(start - sizeof(Block_t)), &got, sizeof(Block_t));

		return (void*)start;
	}
}

void kfree(void *addr)
{
	Block_t blk;
	memcpy(&blk, (void*)((uint32_t)addr - sizeof(Block_t)), sizeof(Block_t));

	if (blk.start < KERNEL_HEAP_BEGIN || blk.start + blk.size > KERNEL_HEAP_END)
		panic("trting to free invalid memory: addr=%p", addr);

	uint32_t free_start = blk.start,
			 free_end = blk.start + blk.size;
	// memory address range to be freed later

	Rbt<Block_start_t>::Node *ptr = tree_start.find_le(blk);
	Block_t got;
	
	if (ptr)
	{
		got = ptr->get_key();
		if (got.start + got.size > blk.start)
			panic("trting to free invalid memory: addr=%p", addr);

		if (got.start + got.size == blk.start)
		{
			free_start = max(free_start & 0xFFFFF000, got.start);

			tree_start.erase(ptr);
			tree_size.erase(tree_size.find_ge(got));
			blk.start = got.start;
			blk.size += got.size;
		}
	}
	

	ptr = tree_start.find_ge(blk);

	if (ptr)
	{
		got = ptr->get_key();
		if (blk.start + blk.size > got.start)
			panic("trting to free invalid memory: addr=%p", addr);

		if (blk.start + blk.size == got.start)
		{
			free_end = min(get_aligned(free_end, 12), blk.start + blk.size);

			tree_start.erase(ptr);
			tree_size.erase(tree_size.find_ge(got));
			blk.size += got.size;
		}
	}

	tree_start.insert(blk);
	tree_size.insert(blk);

	// free used pages
	Page::current_page_dir->free_interval(get_aligned(free_start, 12), free_end & 0xFFFFF000);
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
	b.start = KERNEL_HEAP_BEGIN;
	b.size = KERNEL_HEAP_END - KERNEL_HEAP_BEGIN;
	tree_size.insert(b);
	tree_start.insert(b);

	for (uint32_t i = KERNEL_HEAP_BEGIN; i < KERNEL_HEAP_END; i += 0x1000)
		Page::current_page_dir->get_page(i, true);
}

void kheap_finish_init()
{
	kheap_finish_init_called = true;
	USER_MEM_LOW = get_aligned(kheap_static_end, 22);
	USER_MEM_HIGH = KERNEL_HEAP_BEGIN - 1;
	Klog::log(Klog::INFO, "kernel heap address range: %p %p", (void*)KERNEL_HEAP_BEGIN, (void*)KERNEL_HEAP_END);
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

#ifdef _DEBUG_BUILD_
template <typename T>
void walk_block(const T &b)
{
	Klog::printf(" * start=0x%x size=0x%x\n", b.start, b.size);
}

void kheap_output_debug_msg()
{
	using namespace Klog;

	push_color(LIGHT_GREEN, BLACK);
	puts("start kernel heap debug output\n");

	printf("number of nodes ever allocated: %d\n", Tree_mm::nstatic_mem / Tree_mm::TREE_NODE_SIZE);
	printf("number of currently ununsed nodes: %d\n", Tree_mm::nfreed);

	puts("unallocated blocks sorted by size:\n");
	tree_size.walk(walk_block<Block_size_t>);
	puts("unallocated blocks sorted by start:\n");
	tree_start.walk(walk_block<Block_start_t>);

	puts("end kernel heap debug output\n");
	pop_color();
}
#endif // _DEBUG_BUILD_

