/*
 * $File: descriptor_table.cpp
 * $Date: Sat Nov 27 21:04:37 2010 +0800
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

// these two functions are defined in loader.s
extern "C" void gdt_flush(Uint32_t);
extern "C" void idt_flush(Uint32_t);

static void init_gdt();
static void init_idt();

// defined in interrupt.s
extern "C" void isr8();
extern "C" void isr9();

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

	for (int i = 0; i < 32; i ++)
		idt_entries[i].set((Uint32_t)isr8, 0x08, 0b10001110);
	idt_entries[9].set((Uint32_t)isr9, 0x08, 0b10001110);

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

