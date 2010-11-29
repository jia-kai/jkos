/*
 * $File: kheap.cpp
 * $Date: Mon Nov 29 11:36:14 2010 +0800
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

#include <kheap.h>

// defined in the linker script
extern "C" Uint32_t kernel_img_end;

static Uint32_t kheap_end = ((Uint32_t)&kernel_img_end) + 1;

Uint32_t kmalloc(Uint32_t size, int palign, Uint32_t *phyaddr)
{
	kheap_end = (((kheap_end - 1) >> palign) + 1) << palign;
	if (*phyaddr)
		*phyaddr = kheap_end;

	Uint32_t ret = kheap_end;
	kheap_end += size;
	return ret;
}

Uint32_t kheap_get_mem_size()
{
	return kheap_end;
}

