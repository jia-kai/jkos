/*
 * $File: base.h
 * $Date: Mon Dec 13 15:21:07 2010 +0800
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

namespace Fs
{
	const int FILENAME_LEN_MAX = 128;
	typedef uint32_t off_t;

	const int 
		SEEK_SET = 0,
		SEEK_CUR = 1,
		SEEK_END = 2;

	enum Fsid_t {FSID_DEVFS, FSID_RAMDISK, FSID_JKFS};

	typedef uint16_t mode_t;
	typedef uint32_t nlink_t;
	struct Stat_t
	{
		mode_t mode;
		nlink_t nlink;
	};

	class Node_file;
	class Node_dir;

	class Node_file
	{
		public:
			virtual Fsid_t get_fs_id() = 0;

			// get the directory the file belongs to
			virtual Node_dir* get_dir();

			virtual off_t seek(off_t offset, int whence);
			virtual ssize_t read(void *buf, size_t cnt);
			virtual ssize_t write(const void *buf, size_t cnt);
			virtual int ioctl(int request, ...);
			virtual int stat(Stat_t *buf);

			virtual void close();
	};

	class Dirent_t
	{
		public:
			virtual Fsid_t get_fs_id() = 0;

			// move to the next directory entry, or return false if reaching the end
			virtual bool next();
			const char *name;
			uint8_t type;
	};

	class Node_dir
	{
		public:
			virtual Fsid_t get_fs_id() = 0;

			virtual Node_file* openf(const char *fname);
			virtual Node_dir* opend(const char *fname);

			virtual Node_file* creat(const char *fname, mode_t mode);
			virtual int chmod(const char *fname, mode_t mode);
			virtual int unlink(const char *fname);

			// link @ftpr to @fname
			virtual int link(Node_file *fptr, const char *fname);

			virtual Node_dir* mkdir(const char *fname, mode_t mode);
			virtual int rmdir(const char *fname);

			// mount the @target (directory or filesystem) here
			virtual int mount(Node_dir *target);

			virtual bool is_empty();

			virtual Dirent_t* list();
			virtual Node_dir* get_par();

			virtual void close();
	};

}

#endif // HEADER_FS_BASE

