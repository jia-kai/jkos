/*
 * $File: signal.h
 * $Date: Mon Dec 20 20:04:50 2010 +0800
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

#ifndef HEADER_SIGNAL
#define HEADER_SIGNAL

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

#endif
