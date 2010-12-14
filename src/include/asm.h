/*
 * $File: asm.h
 * $Date: Tue Dec 14 15:42:28 2010 +0800
 *
 * definitions for asm functions
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

#ifndef HEADER_ASM
#define HEADER_ASM

#define KERNEL_CODE_SELECTOR	0x08
#define KERNEL_DATA_SELECTOR	0x10
#define USER_CODE_SELECTOR		0x18
#define USER_DATA_SELECTOR		0x20

#define TSS_DESCRIPTOR_SELECTOR	0x28


#endif // HEADER_ASM

