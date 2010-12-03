/*
 * $File: kheap.h
 * $Date: Wed Dec 01 21:41:13 2010 +0800
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

#ifndef HEADER_KHEAP
#define HEADER_KHEAP

#include <common.h>

/*
 * allocate a chunk of memory in the kernel heap
 * @size: size in byte
 * @palign: the returned address will be aligned on 2^@palign byte boundries
 */
extern void* kmalloc(Uint32_t size, int palign = 0);

extern void kfree(void *ptr);

/*
 * initialize kernel heap (should only be called by Page::init())
 * @start, @size: start address and size of usable virtual memory
 */
extern void kheap_init(Uint32_t start, Uint32_t size);

#ifdef DEBUG
extern void kheap_output_debug_msg();
#endif

#endif // HEADER_KHEAP

