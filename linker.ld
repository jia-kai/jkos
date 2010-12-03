/*
 * $File: linker.ld
 * $Date: Fri Dec 03 18:56:24 2010 +0800
 *
 * the linker script
 *
 * modified from http://wiki.osdev.org/C%2B%2B_Bare_Bones
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

ENTRY (loader)

SECTIONS
{
	. = 0x100000;

	.text ALIGN(4096) :
	{
		*loader.o(.text*)
		/* 
		 * Multiboot Specification requires the Multiboot header be
		 * contained completely within the first 8192 bytes of the OS image
		 */
		*(.text*)
		*(.gnu.linkonce.t*)
	}

	.rodata ALIGN(4096) :
	{
		*(.rodata*)
		*(.rdata*)
		*(.gnu.linkonce.r*)
	}

	.ctors ALIGN(4096) :
	{
		kheap_start_ctors = .;
		*kheap.o(.ctor*)
		start_ctors = .;
		*(.ctor*)
		end_ctors = .;
	}

	.dtors ALIGN(4096) :
	{
		start_dtors = .;
		*(.dtor*)
		end_dtors = .;
	}
	
	.data ALIGN(4096) :
	{
		*(.data*)
		*(.gnu.linkonce.d*)
	}

	.bss ALIGN(4096) :
	{
		*(.COMMON*)
		*(.bss*)
		*(.gnu.linkonce.b*)
	}

	.eh_frame ALIGN(4096) :
	{
		eh_frame_start = .;
		*(.eh_frame*)
		. += 4;
		eh_frame_end = .;
		. += 4;
	}

	kernel_img_end = .;
	
	/DISCARD/ :
	{
		*(.comment)
	}
}

