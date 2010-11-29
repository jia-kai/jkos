/*
 * $File: common.h
 * $Date: Mon Nov 29 18:53:52 2010 +0800
 *
 * some common definitions
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

#ifndef HEADER_COMMON
#define HEADER_COMMON

typedef char Int8_t;
typedef unsigned char Uint8_t;
typedef short int Int16_t;
typedef unsigned short int Uint16_t;
typedef int Int32_t;
typedef unsigned int Uint32_t;
typedef long long int Int64_t;
typedef unsigned long long int Uint64_t;

#define NULL 0

static const int
	CLOCK_TICK_RATE	=	1193180,
	KERNEL_HZ		=	50;

#endif

