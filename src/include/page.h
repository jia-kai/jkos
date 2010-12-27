/*
 * $File: page.h
 * $Date: Mon Dec 27 23:12:28 2010 +0800
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

#ifndef _HEADER_PAGE_
#define _HEADER_PAGE_

#include <common.h>

namespace Page
{
	struct Directory_entry_t
	{
		uint32_t present	: 1;	// whether this page is present in memory
		uint32_t rw			: 1;	// writable iff it is set
		uint32_t user		: 1;	// accessible by all iff it is set
		uint32_t wrt_thr	: 1;	// write-through caching enabled if set,
		// write-back enabled otherwise
		uint32_t cache_dis	: 1;	// this page will not cached iff it is set
		uint32_t accessed	: 1;	// whether this page has been read or written to
		uint32_t zero		: 6;	// set size to 4kb; other fields not used
		uint32_t addr		: 20;	// page table 4kb aligned physical address
	} __attribute__((packed));

	struct Table_entry_t
	{
		uint32_t present	: 1;	// whether this page is present in memory
		uint32_t rw			: 1;	// writable iff it is set
		uint32_t user		: 1;	// accessible by all iff it is set
		uint32_t wrt_thr	: 1;	// write-through caching enabled if set,
		// write-back enabled otherwise
		uint32_t cache_dis	: 1;	// this page will not cached iff it is set
		uint32_t accessed	: 1;	// whether this page has been read or written to
		uint32_t dirty		: 1;	// whether this page has been written to (not updated by CPU)
		uint32_t zero		: 1;
		uint32_t global		: 1;	// if set, prevents the TLB from updating the address in it's cache
									// if CR3 is reset.
									// Note, that the page global enable bit in
									// CR4 must be set to enable this feature. 
		uint32_t allocable	: 1;	// this page can be allocated iff this field is not zero (used to detect page fault)
		uint32_t alloc_fill	: 2;	// whether this page will be filled with zero on allocation in page fault
		uint32_t addr		: 20;	// physical page frame address


		// allocate a frame for this page
		// if a frame is already allocated, nothing happens
		void alloc(bool user, bool writable);

		// mark this page as allocable, but do not allocate physical memory now
		// if a frame is already allocated, nothing happens
		void lazy_alloc(bool user, bool writable, bool fill_zero);

		void fill_uint32(uint32_t val);

		// free the associated frame, and mark the page as unallocable
		void free();

	} __attribute__((packed));


	/*
	 * On page fault with non-present error:
	 *	if the related page exists and is allocable, 
	 *	a frame will be allocated,
	 *	with fileds except @present and @addr unchanged;
	 *	otherwise the corresponding process is considered
	 *	encountering segmentation fault
	 */


	struct Table_t
	{
		Table_entry_t pages[1024];
	};

	struct Directory_t
	{
		// array of pointers to page tables
		// accessible in virtual memory
		Table_t *tables[1024];

		Directory_entry_t entries[1024];

		// physical address of entries
		uint32_t phyaddr;


		// get the page containing virtual address @addr in this page directory
		// if the corresponding table does not exist:
		//		if @make is true, a new table will be allocated
		//		otherwise NULL is returned
		Table_entry_t *get_page(uint32_t addr, bool make = false);

		// make the page entries containing virtual address [@start, @end) and mark them as allocable
		// @begin and @end must be 4kb aligned
		void lazy_alloc_interval(uint32_t begin, uint32_t end, bool user, bool writable, bool fill_zero);

		// alloc physical frames for page entries containing virtual address [@start, end)
		// @begin and @end must be 4kb aligned
		void alloc_interval(uint32_t begin, uint32_t end, bool user, bool writable);

		// @begin and @end must be 4kb aligned
		void free_interval(uint32_t begin, uint32_t end);

		// load this page directory into the CR3 register
		void enable();

		// get physical address of virtual address @addr
		// if the corresponding page does not exist, 0 is returned
		// if the page has no corresponding frame:
		//		if @alloc is true, a new frame will be allocated
		//		otherwise 0 is returned
		uint32_t get_physical_addr(void *addr, bool alloc = false, bool user = false, bool writable = true);
	};

	/*
	 * initialize paging
	 * @ptr_mbd: pointer to multiboot_info_t structure
	 */
	extern void init(void *ptr_mbd);

	/*
	 * copy a page directory:
	 *		link the kernel tables
	 *		copy other frames (mark as ro, copy-on-write)
	 */
	extern Directory_t *clone_directory(const Directory_t *src);

	// invalidate the TLB entry for page containing memory address @addr
	static inline void invlpg(uint32_t addr)
	{ asm volatile ("invlpg %0" : : "m"(*(char*)addr)); }

	extern Directory_t *current_page_dir;
}

#endif // _HEADER_PAGE_

