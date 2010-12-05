# $File: loader.s
# $Date: Sun Dec 05 19:53:51 2010 +0800
#
# Multiboot loader (doc: http://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
#
# reference: http://wiki.osdev.org/C%2B%2B_Bare_Bones
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

.global loader, gdt_flush, idt_flush, initial_stack_pointer

initial_stack_pointer:
.long 0

loader:
	cli
	mov %esp, initial_stack_pointer
	push %eax						# Multiboot magic number
	push %ebx						# Multiboot data structure

	call kmain

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

