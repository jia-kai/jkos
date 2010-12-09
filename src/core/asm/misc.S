/*
 * $File: misc.S
 * $Date: Thu Dec 09 10:56:28 2010 +0800
 *
 * miscellaneous helper functions in assembly
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

#include <asm.h>

.global read_eip, gdt_flush, idt_flush, tss_flush

read_eip:
	pop %eax
	jmp *%eax

/*
load the GDT
this function is called in descriptor_table.cpp
argument: address of GDT ptr
*/
gdt_flush:
	mov 4(%esp), %eax
	lgdt (%eax)

	mov $KERNEL_DATA_SELECTOR, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	jmp $KERNEL_CODE_SELECTOR, $1f
1:
	ret


/*
load the IDT
this function is called in descriptor_table.cpp
argument: address of IDT ptr
*/
idt_flush:
	mov 4(%esp), %eax
	lidt (%eax)
	ret


/*
load the TSS structure
*/
tss_flush:
	mov $TSS_DESCRIPTOR_SELECTOR | 0x03, %ax
	ltr %ax
	ret

