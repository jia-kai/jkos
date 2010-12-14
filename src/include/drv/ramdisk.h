/*
 * $File: ramdisk.h
 * $Date: Tue Dec 14 17:35:45 2010 +0800
 *
 * RAM disk
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

#ifndef HEADER_RAMDISK
#define HEADER_RAMDISK

#include <fs/base.h>
#include <common.h>

// get a ramdisk file in the momory address [start, end)
// the node is created by new operator and should be freed by delete
Fs::Node_file* ramdisk_get_file_node(uint32_t start, uint32_t end);

#endif
