/*
 * $File: klog.h
 * $Date: Wed Dec 29 20:11:55 2010 +0800
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

#ifndef _HEADER_SCIO_
#define _HEADER_SCIO_

namespace Klog
{
	/*
	 * like the printf in libc
	 * available format characters:
	 *	s, x, d, u, c, %, f, p
	 */
	extern void printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
	extern void puts(const char *str);

	enum Log_level_t
	{
		DEBUG, INFO, ERROR
	};

	extern void log(Log_level_t level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

#ifdef _HEADER_STDARG_
	extern void vprintf(const char *fmt, va_list argp);
#endif

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
	extern void push_color(Color_t forecolor, Color_t backcolor = BLACK);

	// pop the color stack
	// Note: if there is only one color in the stack, nothing happens
	extern void pop_color();

	// initialize and call cls()
	extern void init();
}

#endif // _HEADER_SCIO_

