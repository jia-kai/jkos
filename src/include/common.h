/*
 * $File: common.h
 * $Date: Wed Dec 29 20:16:53 2010 +0800
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

#ifndef _HEADER_COMMON_
#define _HEADER_COMMON_


// type definitions
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int int16_t;
typedef unsigned short int uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int int64_t;
typedef unsigned long long int uint64_t;

typedef uint32_t size_t; // size type
typedef int32_t ssize_t; // signed size type

#define NULL 0



// commonly used functions
extern void _panic_func(const char *file, const char *func, int line,
		const char *fmt, ...) __attribute__((format(printf, 4, 5), noreturn));

#define panic(fmt, args...)  _panic_func(__FILE__, __PRETTY_FUNCTION__, __LINE__, fmt, ## args)

#ifdef _DEBUG_BUILD_
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

#define CLI_SAVE_EFLAGS(_var_) \
asm volatile \
( \
	"pushf\n" \
	"cli\n" \
	"popl %0" \
  : "=g"(_var_) \
)

#define RESTORE_EFLAGS(_var_) \
asm volatile \
( \
	"pushl %0\n" \
	"popf" \
	: : "g"(_var_) \
)

template <typename T>
static inline const T& max(const T &a, const T &b)
{ return a > b ? a : b; }

template <typename T>
static inline const T& min(const T &a, const T &b)
{ return a < b ? a : b; }

static inline uint32_t get_aligned(uint32_t addr, int palign)
{ if (!addr) return 0; return (((addr - 1) >> palign) + 1) << palign; }



// constants
static const uint32_t
	CLOCK_TICK_RATE	=	1193180,
	KERNEL_HZ		=	50,

	// KERNEL_STACK_POS must be page (4kb) aligned, and KERNEL_STACK_SIZE must be a multiple of 4kb
	KERNEL_STACK_POS	= 0xFFFFF000, // beginning (highest) address of kernel stack plus 1
	KERNEL_STACK_SIZE	= 4 * 1024,

	// KERNEL_HEAP_END must lie on page directry (4mb) boundary,
	// and only kernel stack should be above kernel heap
	KERNEL_HEAP_END		= (KERNEL_STACK_POS - KERNEL_STACK_SIZE) & 0xFFC00000,
	KERNEL_HEAP_BEGIN	= KERNEL_HEAP_END - 256 * 1024 * 1024;


// global variables
extern uint32_t
	USER_MEM_LOW, USER_MEM_HIGH; // defined in kheap.cpp

#endif // _HEADER_COMMON_

