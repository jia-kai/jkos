/*
 * $File: cxxsupport.cpp
 * $Date: Fri Dec 03 15:32:47 2010 +0800
 *
 * C++ support functions
 *
 * reference: http://wiki.osdev.org/C_PlusPlus
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
#include <scio.h>
#include <lib/cxxsupport.h>

// defined in the linker script
extern "C" uint32_t start_ctors, end_ctors, start_dtors, end_dtors;


// for pure virtual functions
extern "C" void __cxa_pure_virtual() __attribute__((noreturn));

// atexit functions begin
void *__dso_handle = 0;
extern "C" int __cxa_atexit(void (*destructor)(void *), void *arg, void *dso);
extern "C" void __cxa_finalize(void *f);

struct Atexit_func_entry_t
{
	void (*func)(void *);
	void *arg, *dso_handle;
};

static const int NATEXIT_FUNCS_MAX = 128;

static Atexit_func_entry_t atexit_funcs[NATEXIT_FUNCS_MAX];
static int natexit_funcs;

// atexit functions end

void cxxsupport_init()
{
	for(uint32_t *ptr = &start_ctors; ptr < &end_ctors; ptr ++)
		((void (*)(void))*ptr)();
}

void cxxsupport_finalize()
{
	for(uint32_t *ptr = &start_dtors; ptr < &end_dtors; ptr ++)
		((void (*)(void))*ptr)();
	__cxa_finalize(NULL);
}

int __cxa_atexit(void (*func)(void*), void *arg, void *dso)
{
	if (natexit_funcs == NATEXIT_FUNCS_MAX)
		panic("too many functions registered by __cxa_atexit");
	atexit_funcs[natexit_funcs].func = func;
	atexit_funcs[natexit_funcs].arg = arg;
	atexit_funcs[natexit_funcs].dso_handle = dso;
	natexit_funcs ++;
	return 0;
}

void __cxa_finalize(void *f)
{
	if (!f)
	{
		/*
		 * According to the Itanium C++ ABI, if __cxa_finalize is called without a
		 * function ptr, then it means that we should destroy EVERYTHING MUAHAHAHA!!
		 *
		 * TODO:
		 * Note well, however, that deleting a function from here that contains a __dso_handle
		 * means that one link to a shared object file has been terminated. In other words,
		 * We should monitor this list (optional, of course), since it tells us how many links to 
		 * an object file exist at runtime in a particular application. This can be used to tell 
		 * when a shared object is no longer in use. It is one of many methods, however.
		 **/
		MSG_DEBUG("__cxa_finalize called with f=0");
		for (int i = natexit_funcs; (-- i) >= 0; )
			(*atexit_funcs[i].func)(atexit_funcs[i].arg);
		natexit_funcs = 0;
		return;
	}

	for (int i = natexit_funcs; (-- i) >= 0; )
		if (atexit_funcs[i].func == f)
		{
			(*atexit_funcs[i].func)(atexit_funcs[i].arg);
			for (int j = i; j + 1 < natexit_funcs; j ++)
				atexit_funcs[j] = atexit_funcs[j + 1];
			natexit_funcs --;
		}
}

void __cxa_pure_virtual()
{
	panic("pure virtual function can not be called");
}

extern void *operator new(uint32_t size)
{
	return kmalloc(size);
}
 
extern void *operator new[](uint32_t size)
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

