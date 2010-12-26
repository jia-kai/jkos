/*
 * $File: base.cpp
 * $Date: Sun Dec 26 19:59:25 2010 +0800
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

Fs::Node_dir* Fs::Node_file::get_mount_point() const
{
	return mount_point;
}

void Fs::Node_file::set_mount_point(Node_dir *ptr)
{
	mount_point = ptr;
}

Fs::Node_dir* Fs::Node_file::get_dir()
{
	set_errno(ENOTSUP);
	return NULL;
}

Fs::off_t Fs::Node_file::seek(off_t , int )
{
	ERROR_RETURN(ENOTSUP);
}

ssize_t Fs::Node_file::read(void *, size_t )
{
	ERROR_RETURN(ENOTSUP);
}

ssize_t Fs::Node_file::write(const void *, size_t )
{
	ERROR_RETURN(ENOTSUP);
}

int Fs::Node_file::ioctl(uint32_t, uint32_t)
{
	ERROR_RETURN(ENOTSUP);
}

int Fs::Node_file::stat(Stat_t *)
{
	ERROR_RETURN(ENOTSUP);
}

void Fs::Node_file::close()
{
}

Fs::Node_file::Node_file() : mount_point(NULL)
{
}

Fs::Node_file::~Node_file()
{
}

bool Fs::Dirent_t::next()
{
	set_errno(ENOTSUP);
	return false;
}

Fs::Node_dir::Node_dir() : mount_point(NULL)
{
}

Fs::Node_dir* Fs::Node_dir::get_mount_point() const
{
	return mount_point;
}

void Fs::Node_dir::set_mount_point(Node_dir *ptr)
{
	mount_point = ptr;
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
	ERROR_RETURN(ENOTSUP);
}

int Fs::Node_dir::link(Node_file *, const char *)
{
	ERROR_RETURN(ENOTSUP);
}

int Fs::Node_dir::symlink(const char *, const char *)
{
	ERROR_RETURN(ENOTSUP);
}

Fs::Node_dir* Fs::Node_dir::mkdir(const char *, mode_t )
{
	set_errno(ENOTSUP);
	return NULL;
}

int Fs::Node_dir::rmdir(const char *)
{
	ERROR_RETURN(ENOTSUP);
}

int Fs::Node_dir::mount(Node_dir *)
{
	ERROR_RETURN(ENOTSUP);
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
}

int Fs::Fs::mv(const char *, const char *)
{
	ERROR_RETURN(ENOTSUP);
}

Fs::Node_dir::~Node_dir()
{
}

