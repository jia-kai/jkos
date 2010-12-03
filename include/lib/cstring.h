/*
 * $File: cstring.h
 * $Date: Fri Dec 03 18:13:02 2010 +0800
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

#ifndef HEADER_CSTRING
#define HEADER_CSTRING

#include <common.h>

extern "C" void memset(void *dest, int val, size_t cnt);
extern "C" void memcpy(void *dest, const void *src, size_t cnt);
extern "C" char* strcpy(char *dest, const char *src);
extern "C" size_t strlen(const char *str);

#endif // HEADER_CSTRING

