/*
 * $File: common.h
 * $Date: Fri Dec 03 15:44:50 2010 +0800
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

extern void _panic_func(const char *file, const char *func, int line,
		const char *fmt, ...) __attribute__((format(printf, 4, 5), noreturn));

#define panic(fmt, args...)  _panic_func(__FILE__, __PRETTY_FUNCTION__, __LINE__, fmt, ## args)

#ifdef DEBUG
extern void _kassert_failed(const char *statement, const char *file, int line) __attribute__((noreturn));
#	define kassert(s) \
	do \
	{ \
		if (!(s)) \
			_kassert_failed(# s, __FILE__, __LINE__); \
	} while (0)
#else
#	define kassert(s) do {} while(0)
#endif

template <typename T>
static inline const T& max(const T &a, const T &b)
{ return a > b ? a : b; }

template <typename T>
static inline const T& min(const T &a, const T &b)
{ return a < b ? a : b; }

typedef Uint32_t Size_t; // size type
typedef Uint64_t Ssize_t; // signed size type

static const int
	CLOCK_TICK_RATE	=	1193180,
	KERNEL_HZ		=	50;

#endif // HEADER_COMMON

