/*
 * $File: base.h
 * $Date: Fri Dec 03 15:53:10 2010 +0800
 *
 * file system base class
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

#ifndef HEADER_FS_BASE
#define HEADER_FS_BASE

#include <common.h>

const int FILENAME_LEN_MAX = 128;

typedef uint32_t File_size_t;

class Fs_node
{
protected:
	char name[FILENAME_LEN_MAX];	// filename
	uint32_t
		mask,						// the permission mask
		uid, gid,					// owner user id and group id
		flags;						// node type
	File_size_t length;				// length in bytes

public:
	virtual bool open() = 0;
	virtual Ssize_t read(void *buf, Size_t count) = 0;
	virtual Ssize_t write(const void *buf, Size_t count) = 0;
	virtual void close() = 0;
	virtual ~Fs_node() {}
};

#endif // HEADER_FS_BASE

