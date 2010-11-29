/*
 * $File: scio.h
 * $Date: Mon Nov 29 19:29:18 2010 +0800
 *
 * functions for doing basic screen output and keyboard input
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

#ifndef HEADER_SCIO
#define HEADER_SCIO

namespace Scio
{
	/*
	 * like the printf in libc
	 * available format characters:
	 *	s, x, d, u, c, %, f
	 */
	extern void printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

	// clear screen
	extern void cls();

	enum Color_t
	{
		BLACK, BLUE, GREEN, CYAN,
		RED, MAGENTA, BROWN, LIGHT_GRAY,
		DARK_GRAY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN,
		LIGHT_RED, LIGHT_MAGENTA, LIGHT_BROWN, WHITE
	};

	// push the color onto the color stack
	// the color used is the top of the color stack
	// Note: if the stack is full, the top will be replaced
	extern void push_color(Color_t forecolor, Color_t backcolor);

	// pop the color stack
	// Note: if there is only one color in the stack, nothing happens
	extern void pop_color();

	// initialize and call cls()
	extern void init();
}

// report kernel panic and fall into infinite loop
#define PANIC(_msg_) \
do \
{ \
	Scio::printf("PANIC(%s) at %s:%s:%d\n", _msg_, __FILE__, __PRETTY_FUNCTION__, __LINE__); \
	asm volatile ("cli"); \
	for (; ;); \
} while (0)


#endif // HEADER_SCIO

