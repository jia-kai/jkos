/*
 * $File: loader.S
 * $Date: Wed Dec 08 20:12:28 2010 +0800
 *
 * Multiboot loader (doc: http://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
 *
  reference: http://wiki.osdev.org/C%2B%2B_Bare_Bones
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

#define ASM_FILE
#include <multiboot.h>

FLAGS	=	MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO		/* this is the Multiboot 'flag' field */
CHECKSUM	=	-(MULTIBOOT_HEADER_MAGIC + FLAGS)				/* checksum required */

.text

.balign 4
.long MULTIBOOT_HEADER_MAGIC
.long FLAGS
.long CHECKSUM

.global loader, initial_stack_pointer

initial_stack_pointer:
.long 0

loader:
	cli
	mov %esp, initial_stack_pointer
	push %eax						/* Multiboot magic number */
	push %ebx						/* Multiboot data structure */

	call kmain

	cli
	hang:
	   hlt					/* halt machine if kernel return */
	   jmp   hang

