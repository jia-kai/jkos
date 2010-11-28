/*
 * $File: isr.cpp
 * $Date: Sun Nov 28 20:17:13 2010 +0800
 *
 * interrupt service routines
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

#include <typedef.h>
#include <port.h>
#include <scio.h>

struct Isr_registers_t
{
	Uint32_t
		ds,										// pushed in interrupt.s
		edi, esi, ebp, esp, ebx, edx, ecx, eax,	// Pushed by pusha.
		int_no, err_code,						// interrupt number and error code
		eip, cs, eflags, useresp, ss;			// pushed by CPU
};

extern "C" void isr_handler(Isr_registers_t reg)
{
	Scio::printf("interrupt: 0x%x\n", reg.int_no);
}

extern "C" void irq_handler(Isr_registers_t reg)
{
	static int tick = 0;
	Scio::printf("%dth IRQ: %d\n", ++ tick, reg.int_no - 32);

	// send an EOI (end of interrupt) signal to the PICs
	if (reg.int_no >= 40)
		Port::outb(0xA0, 0x20); // send reset signal to slave
	Port::outb(0x20, 0x20); // send reset signal to master
}

