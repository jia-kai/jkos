/*
 * $File: kheap.cpp
 * $Date: Thu Dec 02 11:54:29 2010 +0800
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
#include <rbtree.h>
#include <common.h>

static Uint32_t kheap_begin, kheap_end;

struct Block_t
{
	// unallocated memory
	Uint32_t start, size;
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
	Uint32_t start, size;
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
	static Uint8_t static_mem[STATIC_MEM_SIZE] __attribute__((aligned(2)));
	static void* freed[STATIC_SIZE];
	static int nstatic_mem, nfreed;

	static void* alloc();
	static void free(void *ptr);
}

static Rbt<Block_t> tree_block(Tree_mm::alloc, Tree_mm::free);
static Rbt<Hole_t> tree_hole(Tree_mm::alloc, Tree_mm::free);

static inline Uint32_t get_aligned(Uint32_t addr, int palign)
{ if (!addr) return 0; return (((addr - 1) >> palign) + 1) << palign; }

void* kmalloc(Uint32_t size, int palign)
{
	Block_t req;
	req.size = size;
	req.start = 0;
	while (1)
	{
		Rbt<Block_t>::Node *ptr = tree_block.find_ge(req);
		if (!ptr)
			panic("can not allocate virtual memory chunk");
		Block_t got = ptr->get_key();
		Uint32_t as = get_aligned(got.start, palign); // aligned start address
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
	req.start = (Uint32_t)addr;
	Rbt<Hole_t>::Node *ptr = tree_hole.find_le(req);

	if (!ptr)
		panic("attempt to free a non-existing memory chunk");

	Hole_t got = ptr->get_key();
	tree_hole.erase(ptr);

	Block_t nblock;
	nblock.start = got.start;
	nblock.size = got.size;

	Hole_t tmp; // the hole adjacent to newly freed block

	ptr = tree_hole.find_le(got);
	if (ptr) // try to merge with the left block
		tmp = ptr->get_key();
	else
		tmp.start = kheap_begin, tmp.size = 0;
	if (tmp.start + tmp.size != nblock.start)
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

	if (nblock.start + nblock.size != tmp.start)
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
}

void kheap_init(Uint32_t start, Uint32_t size)
{
	Block_t b;
	b.start = start;
	b.size = size;
	tree_block.insert(b);

	kheap_begin = start;
	kheap_end = start + size;
}

void* Tree_mm::alloc()
{
	if (nfreed)
		return freed[-- nfreed];

	// TODO: dynamic allocate memory
	if (nstatic_mem + TREE_NODE_SIZE >= STATIC_MEM_SIZE)
		panic("rbt for kernel virtual memory management has run out of memory");

	void *ret = static_mem + nstatic_mem;
	nstatic_mem += TREE_NODE_SIZE;
	return ret;
}

void Tree_mm::free(void *ptr)
{
	freed[nfreed ++] = ptr;
}

#ifdef DEBUG
#include <scio.h>

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

	printf("number of nodes ever allocated: %d\n", Tree_mm::nstatic_mem / Tree_mm::TREE_NODE_SIZE);
	printf("number of currently ununsed nodes: %d\n", Tree_mm::nfreed);

	tree_block.walk(walk_block);
	tree_hole.walk(walk_hole);

}
#endif // DEBUG

