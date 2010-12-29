#include "lib/include/syscall.h"
#include "lib/include/signal.h"

#define NULL 0

#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_copy(d,s)  __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;

#define FLOAT_PRECESION	5

typedef unsigned int uint32_t;

char* strcpy(char *dest, const char *src)
{
	char *ret = dest;
	while ((*(dest ++) = *(src ++)) != 0);
	return ret;
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

void putc(char ch)
{
	static char buf[2];
	buf[0] = ch;
	sys_puts(buf);
}

void vprintf(const char *fmt, va_list argp)
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
				case 'p':
					{
						uint32_t val = (uint32_t)va_arg(argp, void*);
						strcpy(buf_mem, "#0x");
						for (int i = 7; i >= 0; i --)
						{
							int x = (val >> (i << 2)) & 0xF;
							if (x < 10)
								x += '0';
							else x += 'A' - 10;
							buf_mem[10 - i] = (char)x;
						}
						buf_mem[11] = 0;
						buf = buf_mem;
					}
					break;
				default:
					strcpy(buf_mem, "(%?)");
			}

			sys_puts(buf);

		} else putc(*fmt);
		fmt ++;
	}
}

void printf(const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vprintf(fmt, argp);
	va_end(argp);
}

void test_sleep()
{
	sys_fork();
	int pid = sys_getpid();
	if (pid == 0)
		sys_sleep(0, Sigset()); // sleep self

	for (int i = 0; ; i ++)
	{
		if (i == 10)
		{
			if (pid == 1)
				sys_wakeup(0);
			else if (pid == 0)
				sys_sleep(1, Sigset()); // sleep other
		}
		if (i == 20 && pid == 0)
			sys_wakeup(1);
		printf("pid %d: %d\n", pid, i);
		for (int volatile j = 0; j  < 10000000; j ++);
	}
}

void test_fork()
{
	const int NF = 4096;
	static int f[NF];
	f[0] = 2;
	sys_puts("before fork()\n");
	int ret = sys_fork();
	sys_puts("fork() done\n");
	f[1] = ret;
	printf("fork() retured %d\n", f[1]);

	if (f[0] != 2 || f[1] != ret)
		goto error;

	for (int i = 2; i < NF; i ++)
		if (f[i])
			goto error;
	printf("%d: ok\n", sys_getpid());
	for (; ;);

error:
	printf("no\n");
	for (; ;);
}

extern "C" void _start()
{
	printf("hello, user mode!\n");
	test_sleep();
}

