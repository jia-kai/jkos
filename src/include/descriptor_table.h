/*
 * $File: descriptor_table.h
 * $Date: Tue Dec 21 11:36:27 2010 +0800
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
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t limit_high_and_flag;
	uint8_t base_high;

	void set(uint32_t base, uint32_t limit, uint8_t access, uint8_t flag);
} __attribute__((packed));

struct GDT_ptr_t
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));


struct IDT_entry_t
{
	uint16_t offset_low;
	uint16_t selector;
	uint8_t zero;
	uint8_t flag;
	uint16_t offset_high;

	void set(uint32_t offset, uint8_t sel, uint8_t flag);
} __attribute__((packed));

struct IDT_ptr_t
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

struct Isr_registers_t
{
	uint32_t
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
 * initialize descriptor tables (GDT, IDT, TSS)
 */
extern void init_descriptor_tables();

#endif

