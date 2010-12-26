/*
 * $File: elf.h
 * $Date: Sun Dec 26 20:26:09 2010 +0800
 *
 * functions for loading ELFs
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

#ifndef _HEADER_ELF_
#define _HEADER_ELF_

#include <fs/base.h>

// load the elf file @file into current page directory
// return a positive integer telling the entrance address,
// or -1 on error and errno is set accordingly
extern int load_elf(Fs::Node_file *file);

#endif
