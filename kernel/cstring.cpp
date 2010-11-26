/*
 * $File: cstring.cpp
 * $Date: Fri Nov 26 20:31:42 2010 +0800
 *
 * functions for manipulating C-style strings
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

#include <cstring.h>

void memset(void *dest, int val, int cnt)
{
	asm volatile
	(
		"cld\n"
		"rep stosb" : :
		"D"(dest), "a"(val), "c"(cnt)
	);
}

void memcpy(void *dest, const void *src, int cnt)
{
	asm volatile
	(
		"cld\n"
		"rep movsb" : :
		"D"(dest), "S"(src), "c"(cnt)
	);
}

char *strcpy(char *dest, const char *src)
{
	asm volatile
	(
		"1:\n"
		"lodsb\n"
		"stosb\n"
		"cmp $0, %%al\n"
		"jne 1b" : :
		"D"(dest), "S"(src) :
		"al"
	);
	return (char*)dest;
}

