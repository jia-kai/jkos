/*
 * $File: kheap.h
 * $Date: Sat Dec 04 20:25:15 2010 +0800
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
 *
 * Note: before calling kheap_finish_init, the memory will be allocated in static kernel heap
 */
extern void* kmalloc(uint32_t size, int palign = 0);

extern void kfree(void *ptr);

/*
 * initialize kernel heap (should only be called by Page::init())
 */
extern void kheap_init();
extern void kheap_finish_init();

/*
 * get static kernel heap usage
 * (should only be called by Page::init())
 */
extern uint32_t kheap_get_size_pre_init(); 


#ifdef DEBUG
extern void kheap_output_debug_msg();
#endif

#endif // HEADER_KHEAP

