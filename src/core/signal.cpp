/*
 * $File: signal.cpp
 * $Date: Thu Dec 23 19:32:47 2010 +0800
 *
 * functions for manipulating signals
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

#include <signal.h>
#include <lib/cstring.h>

Sigset::Sigset()
{
	this->clear();
}

Sigset::Sigset(const Sigset &s)
{
	memcpy(val, s.val, sizeof(val));
}

Sigset& Sigset::operator = (const Sigset &s)
{
	memcpy(val, s.val, sizeof(val));
	return *this;
}

void Sigset::clear()
{
	memset(val, 0, sizeof(val));
}

void Sigset::fill()
{
	memset(val, -1, sizeof(val));
}

void Sigset::add(int signum)
{
	if (signum >= 0 && signum < NSIGNAL_MAX)
		val[signum >> 5] |= 1 << (signum & 31);
}

void Sigset::del(int signum)
{
	if (signum >= 0 && signum < NSIGNAL_MAX)
		val[signum >> 5] &= ~(1 << (signum & 31));
}

bool Sigset::test(int signum)
{
	if (signum >= 0 && signum < NSIGNAL_MAX)
		return (val[signum >> 5] >> (signum & 31)) & 1;
	return false;
}

