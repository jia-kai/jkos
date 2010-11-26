# $File: loader.s
# $Date: Fri Nov 26 11:21:38 2010 +0800
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

.global loader

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

