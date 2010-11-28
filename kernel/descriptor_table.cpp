/*
 * $File: descriptor_table.cpp
 * $Date: Sun Nov 28 20:18:09 2010 +0800
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
#include <cstring.h>
#include <port.h>

// these two functions are defined in loader.s
extern "C" void gdt_flush(Uint32_t);
extern "C" void idt_flush(Uint32_t);

static void init_gdt();
static void init_idt();
/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
static void remap_PIC(int offset1, int offset2);

// defined in interrupt.s
extern "C" void isr8();
extern "C" void isr9();

extern "C" void irq0();
extern "C" void irq1();
extern "C" void irq2();
extern "C" void irq3();
extern "C" void irq4();
extern "C" void irq5();
extern "C" void irq6();
extern "C" void irq7();
extern "C" void irq8();
extern "C" void irq9();
extern "C" void irq10();
extern "C" void irq11();
extern "C" void irq12();
extern "C" void irq13();
extern "C" void irq14();
extern "C" void irq15();

void init_descriptor_tables()
{
	init_gdt();
	init_idt();
}

void init_gdt()
{
	static GDT_entry_t	gdt_entries[5];
	static GDT_ptr_t	gdt_ptr;

	gdt_ptr.limit = sizeof(gdt_entries) - 1;
	gdt_ptr.base = (Uint32_t)&gdt_entries;

	gdt_entries[0].set(0, 0, 0, 0);

	// kernel code segment
	gdt_entries[1].set(0, 0xFFFFFFFF, 0b10011010, 0b1100);

	// kernel data segment
	gdt_entries[2].set(0, 0xFFFFFFFF, 0b10010010, 0b1100);

	// usermode code segment
	gdt_entries[3].set(0, 0xFFFFFFFF, 0b11111010, 0b1100);

	// usermode data segment
	gdt_entries[4].set(0, 0xFFFFFFFF, 0b11110010, 0b1100);

	gdt_flush((Uint32_t)&gdt_ptr);
}

void init_idt()
{
	static IDT_entry_t	idt_entries[256];
	static IDT_ptr_t	idt_ptr;

	idt_ptr.limit = sizeof(idt_entries) - 1;
	idt_ptr.base = (Uint32_t)&idt_entries;

	memset(idt_entries, 0, sizeof(idt_entries));

	const Uint32_t
		IDT_SEL = 0x08,
		IDT_FLAG = 0b10001110;
	for (int i = 0; i < 32; i ++)
		idt_entries[i].set((Uint32_t)isr8, IDT_SEL, IDT_FLAG);
	idt_entries[9].set((Uint32_t)isr9, IDT_SEL, IDT_FLAG);

	remap_PIC(32, 40);

#define ADD_IRQ(n) \
	idt_entries[32 + n].set((Uint32_t)irq##n, IDT_SEL, IDT_FLAG)

	ADD_IRQ(0); ADD_IRQ(1); ADD_IRQ(2); ADD_IRQ(3);
	ADD_IRQ(4); ADD_IRQ(5); ADD_IRQ(6); ADD_IRQ(7);
	ADD_IRQ(8); ADD_IRQ(9); ADD_IRQ(10); ADD_IRQ(11);
	ADD_IRQ(12); ADD_IRQ(13); ADD_IRQ(14); ADD_IRQ(15);

#undef ADD_IRQ

	idt_flush((Uint32_t)&idt_ptr);
}

void GDT_entry_t::set(Uint32_t base, Uint32_t limit, Uint8_t access, Uint8_t flag)
{
	this->base_low = base & 0xFFFF;
	this->base_middle = (base >> 16) & 0xFF;
	this->base_high = (base >> 24) & 0xFF;

	this->limit_low = limit & 0xFFFF;
	this->limit_high_and_flag = ((limit >> 16) & 0xF) | ((flag & 0xF) << 4);

	this->access = access;
}

void IDT_entry_t::set(Uint32_t offset, Uint8_t sel, Uint8_t flag)
{
	this->offset_low = offset & 0xFFFF;
	this->offset_high = (offset >> 16) & 0xFFFF;

	this->selector = sel;
	this->zero = 0;

	this->flag = flag;
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

void remap_PIC(int offset1, int offset2)
{
	using namespace Port;

	Uint8_t
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

