# $File: interrupt.s
# $Date: Sat Nov 27 21:05:01 2010 +0800
#
# asm functions for handling interrupts
#
# modified from JamesM's kernel development tutorials
# (http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html)
# 

#
# This file is part of JKOS
# 
# Copyright (C) <2010>  Jiakai <jia.kai66@gmail.com>
# 
# JKOS is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# JKOS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with JKOS.  If not, see <http://www.gnu.org/licenses/>.
# 

.global isr8, isr9

isr8:
	cli
	pushl $0	# err_code
	pushl $0x08	# int_no
	jmp isr_common_stub

isr9:
	cli
	pushl $1	# err_code
	pushl $0x09	# int_no
	jmp isr_common_stub


isr_common_stub:
	pusha

	mov %ds, %ax
	push %eax

	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	call isr_handler	# in isr.cpp

	pop %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	popa
	add $8, %esp
	sti
	iret

