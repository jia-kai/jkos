/*
 * $File: user.h
 * $Date: Thu Dec 23 16:21:04 2010 +0800
 *
 * user and user permission management
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

#ifndef _HEADER_USER_
#define _HEADER_USER_

#include <types.h>

namespace User
{
	enum Cap_t
	{
		CAP_KILL
	};
	int cap_test(uid_t uid, Cap_t cap);
}

#endif
