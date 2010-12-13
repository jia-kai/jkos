/*
 * $File: base.cpp
 * $Date: Mon Dec 13 15:51:43 2010 +0800
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

#include <fs/base.h>
#include <errno.h>

Fs::Node_dir* Fs::Node_file::get_dir()
{
	set_errno(ENOTSUP);
	return NULL;
}

Fs::off_t Fs::Node_file::seek(off_t , int )
{
	set_errno(ENOTSUP);
	return -1;
}

ssize_t Fs::Node_file::read(void *, size_t )
{
	set_errno(ENOTSUP);
	return -1;
}

ssize_t Fs::Node_file::write(const void *, size_t )
{
	set_errno(ENOTSUP);
	return -1;
}

int Fs::Node_file::ioctl(int , ...)
{
	set_errno(ENOTSUP);
	return -1;
}

int Fs::Node_file::stat(Stat_t *)
{
	set_errno(ENOTSUP);
	return -1;
}

void Fs::Node_file::close()
{
	set_errno(ENOTSUP);
}

bool Fs::Dirent_t::next()
{
	set_errno(ENOTSUP);
	return false;
}

Fs::Node_file* Fs::Node_dir::openf(const char *)
{
	set_errno(ENOTSUP);
	return NULL;
}

Fs::Node_dir* Fs::Node_dir::opend(const char *)
{
	set_errno(ENOTSUP);
	return NULL;
}

Fs::Node_file* Fs::Node_dir::creat(const char *, mode_t )
{
	set_errno(ENOTSUP);
	return NULL;
}

int Fs::Node_dir::chmod(const char *, mode_t )
{
	set_errno(ENOTSUP);
	return NULL;
}

int Fs::Node_dir::unlink(const char *)
{
	set_errno(ENOTSUP);
	return -1;
}

int Fs::Node_dir::link(Node_file *, const char *)
{
	set_errno(ENOTSUP);
	return -1;
}

Fs::Node_dir* Fs::Node_dir::mkdir(const char *, mode_t )
{
	set_errno(ENOTSUP);
	return NULL;
}

int Fs::Node_dir::rmdir(const char *)
{
	set_errno(ENOTSUP);
	return -1;
}

int Fs::Node_dir::mount(Node_dir *)
{
	set_errno(ENOTSUP);
	return -1;
}

bool Fs::Node_dir::is_empty()
{
	set_errno(ENOTSUP);
	return false;
}

Fs::Dirent_t* Fs::Node_dir::list()
{
	set_errno(ENOTSUP);
	return NULL;
}

Fs::Node_dir* Fs::Node_dir::get_par()
{
	set_errno(ENOTSUP);
	return NULL;
}

void Fs::Node_dir::close()
{
	set_errno(ENOTSUP);
}

