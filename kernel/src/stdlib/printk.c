#include <stdlib.h>
#include <stdarg.h>
#include <drivers/graphics/framebuffer.h>

#define MAX_PRINT_SIZE 512
/*
int printk(char *string,...)
{
	va_list ap;

	char buffer[MAX_PRINT_SIZE];
	char int_buffer[10];
	int int_pointer=0;

	int buffer_pointer=0;
	int i;
	int x;

	va_start(ap, string);

	while(1) {
		if (*string==0) break;

		if (*string=='%') {
			string++;
			if (*string=='d') {
				string++;
				x=va_arg(ap, unsigned long);
				int_pointer=9;
				do {
					int_buffer[int_pointer]=(x%10)+'0';
					int_pointer--;
					x/=10;
				} while(x!=0);
				for(i=int_pointer+1;i<10;i++) {
					buffer[buffer_pointer]=int_buffer[i];
					buffer_pointer++;
				}

			}
			else if (*string=='x') {
				string++;
				x=va_arg(ap, unsigned long);
				int_pointer=9;
				do {
					int_buffer[int_pointer]=(x&0xF)+'0';
					if (int_buffer[int_pointer] > '9') int_buffer[int_pointer] += 7;
					int_pointer--;
					x>>=4;
				} while(x!=0);
				for(i=int_pointer+1;i<10;i++) {
					buffer[buffer_pointer]=int_buffer[i];
					buffer_pointer++;
				}
			} else if (*string=='c') {
				string++;
				x=va_arg(ap, unsigned long);
				buffer[buffer_pointer]=x;
				buffer_pointer++;
			} else if (*string=='s') {
				char *s;
				string++;
				s=(char *)va_arg(ap, long);
				while(*s) {
					buffer[buffer_pointer]=*s;
					s++;
					buffer_pointer++;
				}
			}
		}
		else {
			buffer[buffer_pointer]=*string;
			buffer_pointer++;
			string++;
		}
		if (buffer_pointer==MAX_PRINT_SIZE-1) break;
	}

	va_end(ap);

	uart_write((unsigned char *)buffer,buffer_pointer);

	return buffer_pointer;
}

int print_screen(struct chr_dat dat, char *string,...)
{
	va_list ap;

	char buffer[MAX_PRINT_SIZE];
	char int_buffer[10];
	int int_pointer=0;

	int buffer_pointer=0;
	int i;
	int x;

	va_start(ap, string);

	while(1) {
		if (*string==0) break;

		if (*string=='%') {
			string++;
			if (*string=='d') {
				string++;
				x=va_arg(ap, unsigned long);
				int_pointer=9;
				do {
					int_buffer[int_pointer]=(x%10)+'0';
					int_pointer--;
					x/=10;
				} while(x!=0);
				for(i=int_pointer+1;i<10;i++) {
					buffer[buffer_pointer]=int_buffer[i];
					buffer_pointer++;
				}

			}
			else if (*string=='x') {
				string++;
				x=va_arg(ap, int);
				int_pointer=9;
				do {
					int_buffer[int_pointer]=(x&0xF)+'0';
					if (int_buffer[int_pointer] > '9') int_buffer[int_pointer] += 7;
					int_pointer--;
					x>>=4;
				} while(x!=0);
				for(i=int_pointer+1;i<10;i++) {
					buffer[buffer_pointer]=int_buffer[i];
					buffer_pointer++;
				}
			} else if (*string=='c') {
				string++;
				x=va_arg(ap, unsigned long);
				buffer[buffer_pointer]=x;
				buffer_pointer++;
			} else if (*string=='s') {
				char *s;
				string++;
				s=(char *)va_arg(ap, long);
				while(*s) {
					buffer[buffer_pointer]=*s;
					s++;
					buffer_pointer++;
				}
			}
		}
		else {
			buffer[buffer_pointer]=*string;
			buffer_pointer++;
			string++;
		}
		if (buffer_pointer==MAX_PRINT_SIZE-1) break;
	}
	buffer[buffer_pointer] = '\0';

	va_end(ap);

	draw_string(dat.x,dat.y,buffer,dat.attr);

	return buffer_pointer;
}

*/
#if 0

int main(int argc, char **argv) {


	printk("Hello %d %x World!\n",4321,0xdec25);

	return 0;
}

#endif

static void screen_outputchar(struct chr_dat *dat, char c)
{
	if (c == '\r') {
        dat->x = dat->x0;
    } else if(c == '\n') {
        dat->x = dat->x0; dat->y += FONT_HEIGHT;
    } else {
		draw_char(c, dat->x, dat->y, dat->attr);
        dat->x += FONT_WIDTH;
    }
}

static void simple_outputchar(char c)
{
	uart_putc(c);
}

enum flags {
	PAD_ZERO	= 1,
	PAD_RIGHT	= 2,
};

static int screen_prints(struct chr_dat *dat, const char *string, int width, int flags)
{
	int pc = 0, padchar = ' ';

	if (width > 0) {
		int len = 0;
		const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (flags & PAD_ZERO)
			padchar = '0';
	}
	if (!(flags & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			screen_outputchar(dat, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		screen_outputchar(dat, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		screen_outputchar(dat, padchar);
		++pc;
	}

	return pc;
}

static int prints(const char *string, int width, int flags)
{
	int pc = 0, padchar = ' ';

	if (width > 0) {
		int len = 0;
		const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (flags & PAD_ZERO)
			padchar = '0';
	}
	if (!(flags & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			simple_outputchar(padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		simple_outputchar(*string);
		++pc;
	}
	for ( ; width > 0; --width) {
		simple_outputchar(padchar);
		++pc;
	}

	return pc;
}

#define PRINT_BUF_LEN 64

static int screen_outputi(struct chr_dat *dat, long long i, int base, int sign, int width, int flags, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	char *s;
	int t, neg = 0, pc = 0;
	unsigned long long u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return screen_prints(dat, print_buf, width, flags);
	}

	if (sign && base == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % base;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= base;
	}

	if (neg) {
		if( width && (flags & PAD_ZERO) ) {
			screen_outputchar (dat, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + screen_prints (dat, s, width, flags);
}

static int simple_outputi(long long i, int base, int sign, int width, int flags, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	char *s;
	int t, neg = 0, pc = 0;
	unsigned long long u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints(print_buf, width, flags);
	}

	if (sign && base == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % base;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= base;
	}

	if (neg) {
		if( width && (flags & PAD_ZERO) ) {
			simple_outputchar ('-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (s, width, flags);
}

int print_screen(struct chr_dat *dat, char *format, ...)
{
	int width, flags;
	int pc = 0;
	char scr[2];
	va_list ap;
	union {
		char c;
		char *s;
		int i;
		unsigned int u;
		long li;
		unsigned long lu;
		long long lli;
		unsigned long long llu;
		short hi;
		unsigned short hu;
		signed char hhi;
		unsigned char hhu;
		void *p;
	} u;

	va_start(ap, format);

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = flags = 0;
			if (*format == '\0')
				break;
			if (*format == '%')
				goto out;
			if (*format == '-') {
				++format;
				flags = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				flags |= PAD_ZERO;
			}
			if (*format == '*') {
				width = va_arg(ap, int);
				format++;
			} else {
				for ( ; *format >= '0' && *format <= '9'; ++format) {
					width *= 10;
					width += *format - '0';
				}
			}
			switch (*format) {
				case('d'):
					u.i = va_arg(ap, int);
					pc += screen_outputi(dat, u.i, 10, 1, width, flags, 'a');
					break;

				case('u'):
					u.u = va_arg(ap, unsigned int);
					pc += screen_outputi(dat, u.u, 10, 0, width, flags, 'a');
					break;

				case('x'):
					u.u = va_arg(ap, unsigned int);
					pc += screen_outputi(dat, u.u, 16, 0, width, flags, 'a');
					break;

				case('X'):
					u.u = va_arg(ap, unsigned int);
					pc += screen_outputi(dat, u.u, 16, 0, width, flags, 'A');
					break;

				case('c'):
					u.c = va_arg(ap, int);
					scr[0] = u.c;
					scr[1] = '\0';
					pc += screen_prints(dat, scr, width, flags);
					break;

				case('s'):
					u.s = va_arg(ap, char *);
					pc += screen_prints(dat, u.s ? u.s : "(null)", width, flags);
					break;
				case('l'):
					++format;
					switch (*format) {
						case('d'):
							u.li = va_arg(ap, long);
							pc += screen_outputi(dat, u.li, 10, 1, width, flags, 'a');
							break;

						case('u'):
							u.lu = va_arg(ap, unsigned long);
							pc += screen_outputi(dat, u.lu, 10, 0, width, flags, 'a');
							break;

						case('x'):
							u.lu = va_arg(ap, unsigned long);
							pc += screen_outputi(dat, u.lu, 16, 0, width, flags, 'a');
							break;

						case('X'):
							u.lu = va_arg(ap, unsigned long);
							pc += screen_outputi(dat, u.lu, 16, 0, width, flags, 'A');
							break;

						case('l'):
							++format;
							switch (*format) {
								case('d'):
									u.lli = va_arg(ap, long long);
									pc += screen_outputi(dat, u.lli, 10, 1, width, flags, 'a');
									break;

								case('u'):
									u.llu = va_arg(ap, unsigned long long);
									pc += screen_outputi(dat, u.llu, 10, 0, width, flags, 'a');
									break;

								case('x'):
									u.llu = va_arg(ap, unsigned long long);
									pc += screen_outputi(dat, u.llu, 16, 0, width, flags, 'a');
									break;

								case('X'):
									u.llu = va_arg(ap, unsigned long long);
									pc += screen_outputi(dat, u.llu, 16, 0, width, flags, 'A');
									break;

								default:
									break;
							}
							break;
						default:
							break;
					}
					break;
				case('h'):
					++format;
					switch (*format) {
						case('d'):
							u.hi = va_arg(ap, int);
							pc += screen_outputi(dat, u.hi, 10, 1, width, flags, 'a');
							break;

						case('u'):
							u.hu = va_arg(ap, unsigned int);
							pc += screen_outputi(dat, u.lli, 10, 0, width, flags, 'a');
							break;

						case('x'):
							u.hu = va_arg(ap, unsigned int);
							pc += screen_outputi(dat, u.lli, 16, 0, width, flags, 'a');
							break;

						case('X'):
							u.hu = va_arg(ap, unsigned int);
							pc += screen_outputi(dat, u.lli, 16, 0, width, flags, 'A');
							break;

						case('h'):
							++format;
							switch (*format) {
								case('d'):
									u.hhi = va_arg(ap, int);
									pc += screen_outputi(dat, u.hhi, 10, 1, width, flags, 'a');
									break;

								case('u'):
									u.hhu = va_arg(ap, unsigned int);
									pc += screen_outputi(dat, u.lli, 10, 0, width, flags, 'a');
									break;

								case('x'):
									u.hhu = va_arg(ap, unsigned int);
									pc += screen_outputi(dat, u.lli, 16, 0, width, flags, 'a');
									break;

								case('X'):
									u.hhu = va_arg(ap, unsigned int);
									pc += screen_outputi(dat, u.lli, 16, 0, width, flags, 'A');
									break;

								default:
									break;
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
		}
		else {
out:
			screen_outputchar (dat, *format);
			++pc;
		}
	}
	va_end(ap);
	return pc;
}

int printk(char *format, ...)
{
	int width, flags;
	int pc = 0;
	char scr[2];
	va_list ap;
	union {
		char c;
		char *s;
		int i;
		unsigned int u;
		long li;
		unsigned long lu;
		long long lli;
		unsigned long long llu;
		short hi;
		unsigned short hu;
		signed char hhi;
		unsigned char hhu;
		void *p;
	} u;

	va_start(ap, format);

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = flags = 0;
			if (*format == '\0')
				break;
			if (*format == '%')
				goto out;
			if (*format == '-') {
				++format;
				flags = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				flags |= PAD_ZERO;
			}
			if (*format == '*') {
				width = va_arg(ap, int);
				format++;
			} else {
				for ( ; *format >= '0' && *format <= '9'; ++format) {
					width *= 10;
					width += *format - '0';
				}
			}
			switch (*format) {
				case('d'):
					u.i = va_arg(ap, int);
					pc += simple_outputi(u.i, 10, 1, width, flags, 'a');
					break;

				case('u'):
					u.u = va_arg(ap, unsigned int);
					pc += simple_outputi(u.u, 10, 0, width, flags, 'a');
					break;

				case('x'):
					u.u = va_arg(ap, unsigned int);
					pc += simple_outputi(u.u, 16, 0, width, flags, 'a');
					break;

				case('X'):
					u.u = va_arg(ap, unsigned int);
					pc += simple_outputi(u.u, 16, 0, width, flags, 'A');
					break;

				case('c'):
					u.c = va_arg(ap, int);
					scr[0] = u.c;
					scr[1] = '\0';
					pc += prints(scr, width, flags);
					break;

				case('s'):
					u.s = va_arg(ap, char *);
					pc += prints(u.s ? u.s : "(null)", width, flags);
					break;
				case('l'):
					++format;
					switch (*format) {
						case('d'):
							u.li = va_arg(ap, long);
							pc += simple_outputi(u.li, 10, 1, width, flags, 'a');
							break;

						case('u'):
							u.lu = va_arg(ap, unsigned long);
							pc += simple_outputi(u.lu, 10, 0, width, flags, 'a');
							break;

						case('x'):
							u.lu = va_arg(ap, unsigned long);
							pc += simple_outputi(u.lu, 16, 0, width, flags, 'a');
							break;

						case('X'):
							u.lu = va_arg(ap, unsigned long);
							pc += simple_outputi(u.lu, 16, 0, width, flags, 'A');
							break;

						case('l'):
							++format;
							switch (*format) {
								case('d'):
									u.lli = va_arg(ap, long long);
									pc += simple_outputi(u.lli, 10, 1, width, flags, 'a');
									break;

								case('u'):
									u.llu = va_arg(ap, unsigned long long);
									pc += simple_outputi(u.llu, 10, 0, width, flags, 'a');
									break;

								case('x'):
									u.llu = va_arg(ap, unsigned long long);
									pc += simple_outputi(u.llu, 16, 0, width, flags, 'a');
									break;

								case('X'):
									u.llu = va_arg(ap, unsigned long long);
									pc += simple_outputi(u.llu, 16, 0, width, flags, 'A');
									break;

								default:
									break;
							}
							break;
						default:
							break;
					}
					break;
				case('h'):
					++format;
					switch (*format) {
						case('d'):
							u.hi = va_arg(ap, int);
							pc += simple_outputi(u.hi, 10, 1, width, flags, 'a');
							break;

						case('u'):
							u.hu = va_arg(ap, unsigned int);
							pc += simple_outputi(u.lli, 10, 0, width, flags, 'a');
							break;

						case('x'):
							u.hu = va_arg(ap, unsigned int);
							pc += simple_outputi(u.lli, 16, 0, width, flags, 'a');
							break;

						case('X'):
							u.hu = va_arg(ap, unsigned int);
							pc += simple_outputi(u.lli, 16, 0, width, flags, 'A');
							break;

						case('h'):
							++format;
							switch (*format) {
								case('d'):
									u.hhi = va_arg(ap, int);
									pc += simple_outputi(u.hhi, 10, 1, width, flags, 'a');
									break;

								case('u'):
									u.hhu = va_arg(ap, unsigned int);
									pc += simple_outputi(u.lli, 10, 0, width, flags, 'a');
									break;

								case('x'):
									u.hhu = va_arg(ap, unsigned int);
									pc += simple_outputi(u.lli, 16, 0, width, flags, 'a');
									break;

								case('X'):
									u.hhu = va_arg(ap, unsigned int);
									pc += simple_outputi(u.lli, 16, 0, width, flags, 'A');
									break;

								default:
									break;
							}
							break;
						default:
							break;
					}
					break;
				default:
					break;
			}
		}
		else {
out:
			simple_outputchar (*format);
			++pc;
		}
	}
	va_end(ap);
	return pc;
}