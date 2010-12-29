/*
 * $File: syscall.S
 * $Date: Wed Dec 29 20:19:43 2010 +0800
 *
 * isr0x80 handler
 *
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

.extern syscall_func_addr
/* defined in syscall.cpp */

.global isr0x80

isr0x80:
	/* we do not cli so that the scheduler can still work in a system call */

	cmp $NR_SYSCALLS, %eax
	jge done

	pushl %eax

	xor %eax, %eax
	mov %ds, %ax
	pushl %eax

	mov $KERNEL_DATA_SELECTOR, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	/* push syscall arguments */
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx

	movl 4 * 6(%esp), %eax	/* restore original eax */
	call *syscall_func_addr(, %eax, 4)

	movl 4 * 5(%esp), %ebx
	mov %bx, %ds
	mov %bx, %es
	mov %bx, %fs
	mov %bx, %gs

	addl $4 * 7, %esp

done:
	iret
