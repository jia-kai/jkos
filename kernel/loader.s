# $File: loader.s
# $Date: Sun Nov 28 20:04:20 2010 +0800
#
# Multiboot loader (doc: http://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
#
# modified from http://wiki.osdev.org/C%2B%2B_Bare_Bones
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

ALIGN	=	1 << 0		# align loaded modules on page boundaries
MEMINFO	=	1 << 1		# provide memory map
FLAGS	=	ALIGN | MEMINFO		# this is the Multiboot 'flag' field
MAGIC	=	0x1BADB002			# 'magic number' lets bootloader find the header
CHECKSUM	=	-(MAGIC + FLAGS)	# checksum required

.text

.balign 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.global loader, gdt_flush, idt_flush

# reserve initial kernel stack space
STACKSIZE	=	0x4000			# that is, 16k.
.comm stack, STACKSIZE, 32		# reserve 16k stack on a quadword boundary

loader:
	mov $(stack + STACKSIZE), %esp	# set up the stack
	push %eax						# Multiboot magic number
	push %ebx						# Multiboot data structure

	mov $start_ctors, %ebx
	jmp 2f					# call static constructors

1:
	call *(%ebx)
	add $4, %ebx

2:
	cmp $end_ctors, %ebx
	jb 1b

	call  kmain				# call kernel

	mov $start_dtors, %ebx
	jmp 4f					# call static destructors

3:
	call *(%ebx)
	add $4, %ebx

4:
	cmp $end_dtors, %ebx
	jb 3b

	cli
	hang:
	   hlt					# halt machine if kernel return
	   jmp   hang


# load the GDT
# this function is called in descriptor_table.cpp
# argument: address of GDT ptr
gdt_flush:
	mov 4(%esp), %eax
	lgdt (%eax)

	mov $0x10, %ax		# 0x10 is the offset of kernel data segment in the GDT
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	jmp $0x08, $1f		# 0x08 is the offset of kernel code segment in the GDT
1:
	ret


# load the IDT
# this function is called in descriptor_table.cpp
# argument: address of IDT ptr
idt_flush:
	mov 4(%esp), %eax
	lidt (%eax)
	ret

