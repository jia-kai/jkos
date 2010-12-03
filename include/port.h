/*
 * $File: port.h
 * $Date: Sun Nov 28 10:53:17 2010 +0800
 *
 * functions for doing port I/O
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

#ifndef HEADER_PORT
#define HEADER_PORT

#include <common.h>

namespace Port
{
	static inline void outb(Uint16_t port, Uint8_t val)
	{
		asm volatile("outb %0, %1" : : "a"(val), "Nd" (port));
	}

	static inline Uint8_t inb(Uint16_t port)
	{
		Uint8_t ret;
		asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
		return ret;
	}

	static inline void wait()
	{
		asm volatile("outb %%al, $0x80" : : "a"(0));
		// port 0x80 is used for 'checkpoints' during POST.
		// linux kernel seems to think it's free for use :-/
	}
}

#endif // HEADER_PORT

