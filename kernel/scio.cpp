/*
 * $File: scio.cpp
 * $Date: Thu Dec 02 11:07:14 2010 +0800
 *
 * functions for doing basic screen output and keyboard input
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

#include <common.h>
#include <stdarg.h>
#include <scio.h>
#include <cstring.h>
#include <port.h>

const int COLOR_STACK_SIZE = 32,
	  NCOL = 80, NROW = 25,
	  FLOAT_PRECESION = 4;

static volatile Uint8_t *videomem;
static bool video_monochrome;
static Uint8_t color_stack[COLOR_STACK_SIZE];
static int ncolor_stack,
		   xpos, ypos;

static inline void putc(char ch);
static inline void move_cursor();

static const char *u2s(unsigned n, int base);

void Scio::init()
{
	char c = (*(volatile Uint16_t*)0x410) & 0x30;
	if (c == 0x30)
	{
		videomem = (Uint8_t*)0xB0000;
		video_monochrome = true;
	} else
	{
		videomem = (Uint8_t*)0xB8000;
		video_monochrome = false;
	}
	push_color(LIGHT_GRAY, BLACK);
	cls();
	printf("video intialized. mode: %s\n", video_monochrome ? "monochrome" : "color");
}


void Scio::push_color(Color_t forecolor, Color_t backcolor)
{
	if (ncolor_stack == COLOR_STACK_SIZE)
		ncolor_stack --;

	if (video_monochrome)
		color_stack[ncolor_stack ++] = forecolor == backcolor ? 0 : 0x07;
	else
		color_stack[ncolor_stack ++] = (Uint8_t)(((Uint8_t)backcolor) << 4 | ((Uint8_t)forecolor));
}

void Scio::pop_color()
{
	if (ncolor_stack > 1)
		ncolor_stack --;
}

void Scio::printf(const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vprintf(fmt, argp);
	va_end(argp);
}

void Scio::puts(const char *str)
{
	while (*str)
		putc(*(str ++));
	move_cursor();
}

void Scio::vprintf(const char *fmt, va_list argp)
{
	static char buf_mem[128];
	const char *buf;

	while (*fmt)
	{
		if (*fmt == '%')
		{
			switch (*(++ fmt))
			{
				case 's':
					buf = va_arg(argp, const char *);
					if (buf == NULL)
					{
						strcpy(buf_mem, "(null)");
						buf = buf_mem;
					}
					break;
				case 'x':
					buf = u2s(va_arg(argp, unsigned), 16);
					break;
				case 'd':
					{
						int n = va_arg(argp, int);
						if (n < 0)
						{
							putc('-');
							n = -n;
						}
						buf = u2s(n, 10);
					}
					break;
				case 'u':
					buf = u2s(va_arg(argp, unsigned), 10);
					break;
				case 'c':
					buf_mem[0] = (char)va_arg(argp, int);
					buf_mem[1] = 0;
					buf = buf_mem;
					break;
				case '%':
					strcpy(buf_mem, "%");
					buf = buf_mem;
					break;
				case 'f':
					{
						double f = va_arg(argp, double);
						int n = 0;
						if (f < 0)
						{
							putc('-');
							f = -f;
						}
						if (f < 1)
							buf_mem[n ++] = '0';
						else
						{
							double f1 = f;
							while (f >= 1)
							{
								f /= 10;
								buf_mem[n ++] = (char)((int)((f - (int)f) * 10) + '0');
							}
							f = f1;
						}
						for (int i = n / 2 - 1; i >= 0; i --)
						{
							char tmp = buf_mem[i];
							buf_mem[i] = buf_mem[n - 1 - i];
							buf_mem[n - 1 - i] = tmp;
						}
						buf_mem[n ++] = '.';
						for (int i = 0; i < FLOAT_PRECESION; i ++)
						{
							buf_mem[n ++] = (char)((int)((f - (int)f) * 10) + '0');
							f *= 10;
						}
						buf_mem[n ++] = 0;
						buf = buf_mem;
					}
					break;
				default:
					strcpy(buf_mem, "(%?)");
			}

			while (*buf)
				putc(*(buf ++));

		} else putc(*fmt);
		fmt ++;
	}
	move_cursor();
}

void Scio::cls()
{
	xpos = ypos = 0;
	for (int i = 0, j = NCOL * NROW * 2; i < j; i += 2)
	{
		videomem[i] = ' ';
		videomem[i + 1] =  color_stack[ncolor_stack - 1];
	}
}

void putc(char ch)
{
	if (ch < 0)
		return;
	if (ch == '\r')
	{
		xpos = 0;
		return;
	}
	if (ch != '\n')
	{
		int p = (xpos + ypos * NCOL) * 2;
		videomem[p] = ch;
		videomem[p + 1] = color_stack[ncolor_stack - 1];
		xpos ++;
	}

	if (ch == '\n' || xpos == NCOL)
	{
		xpos = 0;
		ypos ++;
		if (ypos == NROW)
		{
			const int csize = NCOL * 2, // column size
				  bsize = (NROW - 1) * NCOL * 2; // block size
			memcpy((char*)videomem, (char*)videomem + csize, bsize);
			for (int i = bsize, j = 0; j < csize; i += 2, j += 2)
			{
				videomem[i] = ' ';
				videomem[i + 1] = color_stack[ncolor_stack - 1];
			}
			ypos = NROW - 1;
		}
	}
}

void move_cursor()
{
	int pos = ypos * NCOL + xpos;
	Port::outb(0x3D4, 0x0E);
	Port::outb(0x3D5, (Uint8_t)((pos >> 8) & 0xFF));
	Port::outb(0x3D4, 0x0F);
	Port::outb(0x3D5, (Uint8_t)(pos & 0xFF));
}

const char *u2s(unsigned n, int base)
{
	static char ret[20];
	if (!n)
		return strcpy(ret, "0");
	int len = 0;
	while (n)
	{
		int d = n % base;
		n /= base;
		ret[len ++] = (char)(d < 10 ? '0' + d : 'a' + d - 10);
	}
	for (int i = len / 2 - 1; i >= 0; i --)
	{
		char tmp = ret[i];
		ret[i] = ret[len - 1 - i];
		ret[len - 1 - i] = tmp;
	}
	ret[len] = 0;
	return ret;
}

