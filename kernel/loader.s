# $File: loader.s
# $Date: Sat Nov 27 20:24:36 2010 +0800
#
# this file is used for setting up the Multiboot header - see GRUB docs for details
# Reference: http://wiki.osdev.org/Bare_bones

ALIGN	=	1 << 0		# align loaded modules on page boundaries
MEMINFO	=	1 << 1		# provide memory map
FLAGS	=	ALIGN | MEMINFO		# this is the Multiboot 'flag' field
MAGIC	=	0x1BADB002			# 'magic number' lets bootloader find the header
CHECKSUM	=	-(MAGIC + FLAGS)	# checksum required

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

