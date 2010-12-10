/*
 * $File: descriptor_table.cpp
 * $Date: Thu Dec 09 21:58:02 2010 +0800
 *
 * initialize descriptor tables
 *
 * modified from JamesM's kernel development tutorials
 * (http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html)
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

#include <descriptor_table.h>
#include <lib/cstring.h>
#include <port.h>
#include <scio.h>
#include <asm.h>

// defined in misc.s
extern "C" void gdt_flush(uint32_t);
extern "C" void idt_flush(uint32_t);
extern "C" void tss_flush();

// defined in interrupt.s
extern "C" uint32_t isr_callback_table[256];

static void init_gdt();
static void init_idt();
extern "C" void isr_unhandled(Isr_registers_t reg);

/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
static void remap_PIC(uint8_t offset1, uint8_t offset2);


struct TSS_entry_t
{
	uint32_t
		prev_tss,
		esp0, ss0, // stack pointer and segment to load when entering kernel mode
		esp1, ss1,
		esp2, ss2,
		cr3,
		eip, eflags,
		eax, ecx, edx, ebx,
		esp, ebp,
		esi, edi,
		es, cs, ss, ds, fs, gs,
		ldt;
	uint16_t reserved, iomap_base;
} __attribute__((packed));
static TSS_entry_t	tss_entry;


#define LOOP_ALL_INTERRUPT(func) \
	func(0); func(1); func(2); func(3); func(4); func(5); func(6); func(7); \
	func(8); func(9); func(10); func(11); func(12); func(13); func(14); func(15); \
	func(16); func(17); func(18); func(19); func(20); func(21); func(22); func(23); \
	func(24); func(25); func(26); func(27); func(28); func(29); func(30); func(31); \
	func(32); func(33); func(34); func(35); func(36); func(37); func(38); func(39); \
	func(40); func(41); func(42); func(43); func(44); func(45); func(46); func(47); \
	func(0x80);

#define EXTERN_ISR(n) \
	extern "C" void isr##n()

LOOP_ALL_INTERRUPT(EXTERN_ISR)

#undef EXTERN_ISR

void init_descriptor_tables()
{
	init_gdt();
	init_idt();
}

void init_gdt()
{
	static GDT_entry_t	gdt_entries[6];
	static GDT_ptr_t	gdt_ptr;

	gdt_ptr.limit = sizeof(gdt_entries) - 1;
	gdt_ptr.base = (uint32_t)&gdt_entries;

	gdt_entries[0].set(0, 0, 0, 0);

	// kernel code segment
	gdt_entries[1].set(0, 0xFFFFFFFF, 0b10011010, 0b1100);

	// kernel data segment
	gdt_entries[2].set(0, 0xFFFFFFFF, 0b10010010, 0b1100);

	// usermode code segment
	gdt_entries[3].set(0, 0xFFFFFFFF, 0b11111010, 0b1100);

	// usermode data segment
	gdt_entries[4].set(0, 0xFFFFFFFF, 0b11110010, 0b1100);

	// TSS
	memset(&tss_entry, 0, sizeof(tss_entry));
	tss_entry.ss0 = KERNEL_DATA_SELECTOR;
	tss_entry.cs = KERNEL_CODE_SELECTOR | 0x03;
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = KERNEL_DATA_SELECTOR | 0x03;
	gdt_entries[5].set((uint32_t)&tss_entry, ((uint32_t)&tss_entry) + sizeof(TSS_entry_t), 0b11101001, 0);

	gdt_flush((uint32_t)&gdt_ptr);
	tss_flush();
}

void init_idt()
{
	static IDT_entry_t	idt_entries[256];
	static IDT_ptr_t	idt_ptr;

	idt_ptr.limit = sizeof(idt_entries) - 1;
	idt_ptr.base = (uint32_t)&idt_entries;

	memset(idt_entries, 0, sizeof(idt_entries));

	const uint32_t
		IDT_SEL = KERNEL_CODE_SELECTOR,
		IDT_FLAG = 0b10001110;

	for (int i = 0; i < 256; i ++)
		if (!isr_callback_table[i])
			isr_callback_table[i] = (uint32_t)isr_unhandled;

	remap_PIC(32, 40);

#define ADD_ISR(n) \
	idt_entries[n].set((uint32_t)isr##n, IDT_SEL, IDT_FLAG)

	LOOP_ALL_INTERRUPT(ADD_ISR)

#undef ADD_IRQ

	idt_entries[0x80].set((uint32_t)isr0x80, IDT_SEL, IDT_FLAG | 0b01100000);

	idt_flush((uint32_t)&idt_ptr);
}

void isr_register(int num, Isr_t callback)
{
	if (num >= 0 && num < 256)
		isr_callback_table[num] = (uint32_t)callback;
}

void isr_unhandled(Isr_registers_t reg)
{
	Scio::printf("unhandled interrupt: int_no=0x%x  err_code=0x%x\n",
			reg.int_no, reg.err_code);

	if (reg.int_no >= 32 && reg.int_no <= 47)
	{
		// unhandled IRQ interrupt
		isr_eoi(reg.int_no);
	}

	panic("unhandled interrupt");
}

void GDT_entry_t::set(uint32_t base, uint32_t limit, uint8_t access_, uint8_t flag)
{
	this->base_low = base & 0xFFFF;
	this->base_middle = (base >> 16) & 0xFF;
	this->base_high = (uint8_t)((base >> 24) & 0xFF);

	this->limit_low = limit & 0xFFFF;
	this->limit_high_and_flag = (uint8_t)(((limit >> 16) & 0xF) | ((flag & 0xF) << 4));

	this->access = access_;
}

void IDT_entry_t::set(uint32_t offset, uint8_t sel_, uint8_t flag_)
{
	this->offset_low = offset & 0xFFFF;
	this->offset_high = (uint16_t)((offset >> 16) & 0xFFFF);

	this->selector = sel_;
	this->zero = 0;

	this->flag = flag_;
}

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8 and 70, as configured by default */

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

void remap_PIC(uint8_t offset1, uint8_t offset2)
{
	using namespace Port;

	uint8_t
		a1 = inb(PIC1_DATA),					// save masks
		a2 = inb(PIC2_DATA);

	outb(PIC1_COMMAND, ICW1_INIT + ICW1_ICW4);	// starts the initialization sequence
	wait();
	outb(PIC2_COMMAND, ICW1_INIT + ICW1_ICW4);
	wait();
	outb(PIC1_DATA, offset1);					// define the PIC vectors
	wait();
	outb(PIC2_DATA, offset2);
	wait();
	outb(PIC1_DATA, 4);							// continue initialization sequence
	wait();
	outb(PIC2_DATA, 2);
	wait();

	outb(PIC1_DATA, ICW4_8086);
	wait();
	outb(PIC2_DATA, ICW4_8086);
	wait();

	outb(PIC1_DATA, a1);   // restore saved masks.
	outb(PIC2_DATA, a2);
}

void isr_eoi(int int_no)
{
	if (int_no >= 40)
		Port::outb(PIC2_COMMAND, 0x20); // send reset signal to slave
	Port::outb(PIC1_COMMAND, 0x20); // send reset signal to master
}

void set_kernel_stack(uint32_t addr)
{
	tss_entry.esp0 = addr;
}

