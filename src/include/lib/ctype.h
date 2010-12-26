/*
 * $File: ctype.h
 * $Date: Thu Dec 02 22:00:52 2010 +0800
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

#ifndef _HEADER_CTYPE_
#define _HEADER_CTYPE_

static inline int isascii(int c)
{
	return ((c & ~0x7f) == 0);
}

static inline int iscntrl(int c)
{
	return ((c) < ' ') || ((c) > 126);
}

static inline int isdigit(int c)
{
	return ((c) >= '0') && ((c) <= '9');
}

static inline int isgraph(int c)
{
	return ((c) > ' ') && ((c) <= 126);
}

static inline int islower(int c)
{
	return (c >= 'a') && (c <= 'z');
}

static inline int isprint(int c)
{
	return ((c) >= ' ') && ((c) <= 126);
}

static inline int isspace(int c)
{
	return ((c) == ' ') || ((c) == '\f')
		|| ((c) == '\n') || ((c) == '\r')
		|| ((c) == '\t') || ((c) == '\v');
}

static inline int isupper(int c)
{
	return (c >= 'A') && (c <= 'Z');
}

static inline int isxdigit(int c)
{
	return isdigit(c) ||
		((c >= 'A') && (c <= 'F')) ||
		((c >= 'a') && (c <= 'f'));
}

static inline int isalpha(int c)
{
	return islower(c) || isupper(c);
}

static inline int isalnum(int c)
{
	return isalpha(c) || isdigit(c);
}

static inline int ispunct(int c)
{
	return isgraph(c) && !isalnum(c);
}

static inline int toascii(int c)
{
	return ((c) & 0x7f);
}

static inline int toupper(int c)
{
	return ((c >= 'a') && (c <= 'z')) ? (c - 'a' + 'A') : c;
}

static inline int tolower(int c)
{
	return ((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 'a') : c;
}

#endif

