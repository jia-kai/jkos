/*
 * $File: ramdisk.cpp
 * $Date: Tue Dec 14 17:19:33 2010 +0800
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

#include <drv/ramdisk.h>
#include <lib/cstring.h>
#include <errno.h>

using namespace Fs;

class Ramdisk_fs: public ::Fs::Fs
{
public:
	Fsid_t get_fs_id() const
	{
		return FSID_RAMDISK;
	}
};

static Ramdisk_fs ramdisk_fs;

class Ramdisk_file_node: public Node_file
{
	uint32_t start, end;
	off_t cur_offset;
	bool closed;
public:
	Ramdisk_file_node(uint32_t s, uint32_t e) :
		start(s), end(e), cur_offset(0), closed(false)
	{}

	::Fs::Fs* get_fs()
	{
		return &ramdisk_fs;
	}

	off_t seek(off_t offset, int whence)
	{
		if (this->closed)
		{
			set_errno(EBADF);
			return -1;
		}
		off_t off1;
		switch (whence)
		{
			case SEEK_SET:
				off1 = offset;
				break;
			case SEEK_CUR:
				off1 = offset + this->cur_offset;
				break;
			case SEEK_END:
				off1 = this->end - this->start + offset;
				break;
			default:
				off1 = -1;
		}
		if (off1 < 0 || off1 >= this->end - this->start)
		{
			set_errno(EINVAL);
			return -1;
		}

		return this->cur_offset = off1;
	}

	ssize_t read(void *buf, size_t cnt)
	{
		if (this->closed)
		{
			set_errno(EBADF);
			return -1;
		}
		cnt = (size_t)min<off_t>(cnt, this->end - this->start - this->cur_offset);

		if (!cnt)
			return 0;

		asm volatile ("pushf\ncli");
		// disable interrupt while writing

		memcpy(buf, (void*)(this->start + this->cur_offset), cnt);
		this->cur_offset += cnt;
		asm volatile ("popf");

		return cnt;
	}

	ssize_t write(const void *buf, size_t cnt)
	{
		if (this->closed)
		{
			set_errno(EBADF);
			return -1;
		}
		cnt = (size_t)min<off_t>(cnt, this->end - this->start - this->cur_offset);

		if (!cnt)
			return 0;

		asm volatile ("pushf\ncli");
		// disable interrupt while writing

		memcpy((void*)(this->start + this->cur_offset), buf, cnt);
		this->cur_offset += cnt;
		asm volatile ("popf");

		return cnt;
	}

	void close()
	{
		this->closed = true;
	}
};

Node_file* ramdisk_get_file_node(uint32_t start, uint32_t end)
{
	return new Ramdisk_file_node(start, end);
}

