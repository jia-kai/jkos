/*
 * $File: common.cpp
 * $Date: Wed Dec 01 20:45:20 2010 +0800
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
#include <stdarg.h>
#include <scio.h>

void _panic_func(const char *file, const char *func, int line, const char *fmt, ...)
{
	Scio::printf("KERNEL PANIC at %s:%s:%d :\n", file, func, line);
	va_list ap;
	va_start(ap, fmt);
	Scio::vprintf(fmt, ap);
	va_end(ap);

	asm volatile ("cli");
	for (; ;);
}

void _kassert_failed(const char *statement, const char *file, int line)
{
	Scio::printf("assertion \"%s\" failed at %s:%d\n",
			statement, file, line);

	asm volatile ("cli");
	for (; ;);
}

