/*
 * $File: kheap.h
 * $Date: Mon Nov 29 11:35:45 2010 +0800
 *
 * manipulate kernel heap
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

#ifndef HEADER_KHEAP
#define HEADER_KHEAP

#include <common.h>

/*
 * allocate a chunk of memory in the kernel heap
 * @size: size in byte
 * @palign: the returned address will be aligned on 2^@palign byte boundries
 * @phyaddr: if not NULL, returns the physical address of allocated memory
 */
extern Uint32_t kmalloc(Uint32_t size, int palign = 0, Uint32_t *phyaddr = 0);

/*
 * allocate a page aligned chunck of memory
 *
 * _a means aligned
 */
static inline Uint32_t kmalloc_a(Uint32_t size)
{
	return kmalloc(size, 12);
}

/*
 * allocate a chunck of memory, and return the physical address
 *
 * _p means physical
 */
static inline Uint32_t kmalloc_p(Uint32_t size, Uint32_t *phyaddr)
{
	return kmalloc(size, 0, phyaddr);
}

static inline Uint32_t kmalloc_ap(Uint32_t size, Uint32_t *phyaddr)
{
	return kmalloc(size, 12, phyaddr);
}

// get the amount of used memory in kernel's virtual address space
extern Uint32_t kheap_get_mem_size();

#endif // HEADER_KHEAP

