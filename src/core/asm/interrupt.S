/*
 * $File: interrupt.S
 * $Date: Wed Dec 08 19:39:50 2010 +0800
 *
 * asm functions for handling interrupts
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

.comm isr_callback_table, 256 * 4

.macro ISR int, err_code
	.global isr\int
	isr\int :
		cli
		.if \err_code == 0
			pushl $0
		.endif
		pushl $\int
		pusha

		xor %eax, %eax
		mov %ds, %ax
		push %eax

		mov $0x10, %ax		/* kernel data segment */
		mov %ax, %ds
		mov %ax, %es
		mov %ax, %fs
		mov %ax, %gs

		movl $isr_callback_table, %eax
		movl 4*\int(%eax), %eax
		call *%eax

		pop %eax
		mov %ax, %ds
		mov %ax, %es
		mov %ax, %fs
		mov %ax, %gs

		popa
		add $8, %esp		/* clear pushed error code and interrupt number */
		sti
		iret
.endm

/* ISR without error code */
.irp n,0,1,2,3,4,5,6,7,9,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
	ISR \n, 0
.endr

/* ISR with error code */
.irp n,8,10,11,12,13,14
	ISR \n, 1
.endr


/* IRQ */
.irp n,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47
	ISR \n, 0
.endr

