/*
 * $File: main.cpp
 * $Date: Sat Nov 27 21:05:13 2010 +0800
 *
 * This file is the main routine of JKOS kernel
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

#include <multiboot.h>
#include <scio.h>
#include <descriptor_table.h>

extern "C" void kmain(Multiboot_info_t* , unsigned int magic)
{
	init_descriptor_tables();
	Scio::init();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		Scio::printf("invalid magic: 0x%x\n", magic);
		return;
	}

	asm volatile ("int $0x08");
	asm volatile ("int $0x09");
	asm volatile ("int $0x08");

	Scio::printf("hello, world!\n");
	for (int volatile i = 0; i < 100000000; i ++);

	for (int i = 1; i < 100; i ++)
	{
		Scio::printf("%s :%d\n", "hello, world!", i);
		Scio::printf("int: 0x%x %u\n", i, i * 2);
		Scio::printf("char: %c %%\n", 'X');
		Scio::printf("double: %f\ndone", 31.41592653589793 / i);
	}
}

