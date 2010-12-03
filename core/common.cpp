/*
 * $File: common.cpp
 * $Date: Fri Dec 03 15:45:03 2010 +0800
 *
 * some common definitions and functions
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

#include <common.h>
#include <lib/stdarg.h>
#include <scio.h>
#include <port.h>

static void die() __attribute__((noreturn));

void _panic_func(const char *file, const char *func, int line, const char *fmt, ...)
{
	Scio::push_color(Scio::RED, Scio::BLACK);
	Scio::printf("KERNEL PANIC at %s:%s:%d :\n", file, func, line);
	va_list ap;
	va_start(ap, fmt);
	Scio::vprintf(fmt, ap);
	va_end(ap);

	die();
}

#ifdef DEBUG
void _kassert_failed(const char *statement, const char *file, int line)
{
	Scio::push_color(Scio::RED, Scio::BLACK);
	Scio::printf("assertion \"%s\" failed at %s:%d\n",
			statement, file, line);

	die();
}
#endif

static void die()
{
	// enable pc speaker
	uint8_t port_0x61_val = Port::inb(0x61) & 0xFC;
	Port::wait();
	Port::outb(0x43, 0xB6);
	uint16_t count = CLOCK_TICK_RATE / 2000;
	Port::outb(0x42, (uint8_t)(count & 0xFF));
	Port::outb(0x42, (uint8_t)(count >> 8));
	Port::outb(0x61, port_0x61_val | 3);

	asm volatile ("cli");
	for (; ;);
}

