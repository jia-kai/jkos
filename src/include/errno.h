/*
 * $File: errno.h
 * $Date: Mon Dec 13 15:46:02 2010 +0800
 *
 * errno definitions
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

#ifndef HEADER_ERRNO
#define HEADER_ERRNO

// defined in task.cpp
extern void set_errno(int errno);

#include <errno-base.h>

#define ENOTSUP		35	/* operation not supported */

#endif
