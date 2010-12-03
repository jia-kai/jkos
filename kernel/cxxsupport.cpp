/*
 * $File: cxxsupport.cpp
 * $Date: Thu Dec 02 19:39:33 2010 +0800
 *
 * C++ support functions
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
#include <kheap.h>

extern "C" void __cxa_pure_virtual() __attribute__((noreturn));

void __cxa_pure_virtual()
{
	panic("pure virtual function can not be called");
}

extern void *operator new(Uint32_t size)
{
	return kmalloc(size);
}
 
extern void *operator new[](Uint32_t size)
{
	return kmalloc(size);
}
 
extern void operator delete(void *p)
{
	kfree(p);
}
 
extern void operator delete[](void *p)
{
	kfree(p);
}

