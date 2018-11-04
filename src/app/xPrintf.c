#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"

#define xPrintf_PutChar uart_char

const char num_table[]="0123456789abcdef";

void vNum2String(unsigned long data,unsigned long base)
{
	char result[33];//32 binary
	signed char i=0;
	while(1)
	{
		result[i++]=num_table[data%base];
		
		data /= base;
		if(data==0)
			break;
	}
	for(;i>=0;i--)
	{
		xPrintf_PutChar(result[i]);
	}
}

unsigned char Isdigit(char c)
{
	if((c >= 0x30) && (c <= 0x39))
		return 1;
	else
		return 0;
}

int atoi(char *str)
{
	int num = 0;;

	while(Isdigit(*str))
	{
		num *= 10;
		num += (*str++) - 0x30;
	}
	return num;
}
static unsigned char buf[128];


// does a more powerful printf (supports %d, %u, %o, %x, %c, %s)
// Supports:
// %d - decimal
// %u - unsigned decimal
// %o - octal
// %x - hex
// %c - character
// %s - strings
// and the width,precision,padding modifiers
// **this printf does not support floating point numbers

int xPrintf(const char *sfmt, ...)
{
	register unsigned char *f, *bp;
	register long l;
	register unsigned long u;
	register int i;
	register int fmt;
	register unsigned char pad = ' ';
	int flush_left = 0, f_width = 0, prec = 2^32-1, hash = 0, do_long = 0;
	int sign = 0;

	va_list ap;
	va_start(ap, sfmt);

	f = (unsigned char *) sfmt;

	for (; *f; f++)
	{
		if (*f != '%')
		{	// not a format character
			// then just output the char
			xPrintf_PutChar(*f);
		}
		else 
		{
			f++;						// if we have a "%" then skip it
			if (*f == '-')
			{
				flush_left = 1;	// minus: flush left
				f++;
			}
            if (*f == '0'
				 || *f == '.')
				{
					// padding with 0 rather than blank
					pad = '0';
					f++;
            }
            if (*f == '*')
				{	// field width
					f_width = va_arg(ap, int);
					f++;
            }
            else if (Isdigit(*f))
				{
					f_width = atoi((char *) f);
					while (Isdigit(*f))
						f++;        // skip the digits
            }
            if (*f == '.')
				{	// precision
					f++;
					if (*f == '*')
					{
						prec = va_arg(ap, int);
						f++;
					}
					else if (Isdigit(*f))
					{
						prec = atoi((char *) f);
						while (Isdigit(*f))
							f++;    // skip the digits
					}
				}
            if (*f == '#')
				{	// alternate form
					hash = 1;
					f++;
            }
            if (*f == 'l')
				{	// long format
					do_long = 1;
					f++;
            }

				fmt = *f;
				bp = buf;
				switch (fmt) {		// do the formatting
				case 'd':			// 'd' signed decimal
					if (do_long)
						l = va_arg(ap, long);
					else
						l = (long) (va_arg(ap, int));
					if (l < 0)
					{
						sign = 1;
						l = -l;
					}
					do	{
						*bp++ = l % 10 + '0';
					} while ((l /= 10) > 0);
					if (sign)
						*bp++ = '-';
					f_width = f_width - (bp - buf);
					if (!flush_left)
						while (f_width-- > 0)
							xPrintf_PutChar(pad);
					for (bp--; bp >= buf; bp--)
						xPrintf_PutChar(*bp);
					if (flush_left)
						while (f_width-- > 0)
							xPrintf_PutChar(' ');
					break;
            case 'o':			// 'o' octal number
            case 'x':			// 'x' hex number
            case 'u':			// 'u' unsigned decimal
					if (do_long)
						u = va_arg(ap, unsigned long);
					else
						u = (unsigned long) (va_arg(ap, unsigned));
					if (fmt == 'u')
					{	// unsigned decimal
						do {
							*bp++ = u % 10 + '0';
						} while ((u /= 10) > 0);
					}
					else if (fmt == 'o')
					{  // octal
						do {
							*bp++ = u % 8 + '0';
						} while ((u /= 8) > 0);
						if (hash)
							*bp++ = '0';
					}
					else if (fmt == 'x')
					{	// hex
						do {
							i = u % 16;
							if (i < 10)
								*bp++ = i + '0';
							else
								*bp++ = i - 10 + 'a';
						} while ((u /= 16) > 0);
						if (hash)
						{
							*bp++ = 'x';
							*bp++ = '0';
						}
					}
					i = f_width - (bp - buf);
					if (!flush_left)
						while (i-- > 0)
							xPrintf_PutChar(pad);
					for (bp--; bp >= buf; bp--)
						xPrintf_PutChar((int) (*bp));
					if (flush_left)
						while (i-- > 0)
							xPrintf_PutChar(' ');
					break;
            case 'c':			// 'c' character
					i = va_arg(ap, int);
					xPrintf_PutChar((int) (i));
					break;
            case 's':			// 's' string
					bp = va_arg(ap, unsigned char *);
					if (!bp)
						bp = (unsigned char *) "(nil)";
					f_width = f_width - strlen((char *) bp);
					if (!flush_left)
						while (f_width-- > 0)
							xPrintf_PutChar(pad);
					for (i = 0; *bp && i < prec; i++)
					{
						xPrintf_PutChar(*bp);
						bp++;
					}
					if (flush_left)
						while (f_width-- > 0)
							xPrintf_PutChar(' ');
					break;
            case '%':			// '%' character
					xPrintf_PutChar('%');
					break;
			}
			flush_left = 0, f_width = 0, prec = 2^32-1, hash = 0, do_long = 0;
			sign = 0;
			pad = ' ';
		}
	}

	va_end(ap);
	return 0;
}
