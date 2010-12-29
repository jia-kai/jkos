/*
 * $File: cstring.h
 * $Date: Wed Dec 29 19:34:59 2010 +0800
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

#ifndef _HEADER_CSTRING_
#define _HEADER_CSTRING_

#include <common.h>

extern void memset(void *dest, int val, size_t cnt);
extern void memcpy(void *dest, const void *src, size_t cnt);
extern char* strcpy(char *dest, const char *src);

#endif // _HEADER_CSTRING_

