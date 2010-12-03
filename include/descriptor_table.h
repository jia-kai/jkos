/*
 * $File: descriptor_table.h
 * $Date: Mon Nov 29 18:55:44 2010 +0800
 *
 * descriptor table structures
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

#ifndef HEADER_DESCRIPTOR_TABLE
#define HEADER_DESCRIPTOR_TABLE

#include <common.h>

// Global Descriptor Table
struct GDT_entry_t
{
	Uint16_t limit_low;
	Uint16_t base_low;
	Uint8_t base_middle;
	Uint8_t access;
	Uint8_t limit_high_and_flag;
	Uint8_t base_high;

	void set(Uint32_t base, Uint32_t limit, Uint8_t access, Uint8_t flag);
} __attribute__((packed));

struct GDT_ptr_t
{
	Uint16_t limit;
	Uint32_t base;
} __attribute__((packed));


struct IDT_entry_t
{
	Uint16_t offset_low;
	Uint16_t selector;
	Uint8_t zero;
	Uint8_t flag;
	Uint16_t offset_high;

	void set(Uint32_t offset, Uint8_t sel, Uint8_t flag);
} __attribute__((packed));

struct IDT_ptr_t
{
	Uint16_t limit;
	Uint32_t base;
} __attribute__((packed));

struct Isr_registers_t
{
	Uint32_t
		ds,										// pushed in interrupt.s
		edi, esi, ebp, esp, ebx, edx, ecx, eax,	// Pushed by pusha.
		int_no, err_code,						// interrupt number and error code
		eip, cs, eflags, useresp, ss;			// pushed by CPU
};

typedef void (*Isr_t)(Isr_registers_t);

#define ISR_GET_NUM_BY_IRQ(n) ((n) + 32)

/*
 * register an interrupt handler
 * num: the interrupt number (between 0 and 47)
 */
extern void isr_register(int num, Isr_t callback);

/*
 * send an EOI (end of interrupt) signal to the PICs
 * int_no: interrupt number
 */
extern void isr_eoi(int int_no);

/*
 * initialize descriptor tables (GDT, IDT)
 */
extern void init_descriptor_tables();

#endif

