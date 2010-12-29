/*
 * $File: signal.h
 * $Date: Wed Dec 29 19:33:53 2010 +0800
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

#ifndef _HEADER_SIGNAL_
#define _HEADER_SIGNAL_

void memset(void *dest, int val, int cnt)
{
	asm volatile
	(
		"cld\n"
		"rep stosb" : :
		"D"(dest), "a"(val), "c"(cnt)
	);
}

void memcpy(void *dest, const void *src, int cnt)
{
	asm volatile
	(
		"cld\n"
		"rep movsb" : :
		"D"(dest), "S"(src), "c"(cnt)
	);
}

typedef unsigned int uint32_t;

const int NSIGNAL_MAX = 32;
class Sigset
{
	uint32_t val[NSIGNAL_MAX / 32];
public:
	Sigset();
	Sigset(const Sigset &s);

	Sigset& operator = (const Sigset &s);

	void clear();
	void fill();
	void add(int signum);
	void del(int signum);
	bool test(int signum);
};

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

#endif
