#include "../include/syscalls.h"
#include "../../kernel/include/process/proc.h"

int serial_write(const void *buff, unsigned int count)
{
    int ret = 0;
    register long r7 __asm__("r7") = SYS_SWRITE;
    register long r0 __asm__("r0") = (long)(buff);
    register long r1 __asm__("r1") = count;

    asm volatile(
        "svc #0\n"
        : "=r"(r0)
        : "r"(r7), "0"(r0), "r"(r1)
        : "memory");
    
    if (r0 != E_NOERR) {
        ret = -r0;
    }

    return ret;
}

unsigned char serial_read(void)
{
    int ret = 0;
    register long r7 __asm__("r7") = SYS_SREAD;
    register long r0 __asm__("r0") = 0;
    
    asm volatile(
        "svc #0\n"
        : "=r"(r0)
        : "r"(r7)
        : "memory");

    ret = r0;

    return ret;
}

int write(FILE *fd, const void *buff, unsigned int count)
{
    int ret = 0;
    register long r7 __asm__("x7") = SYS_FWRITE;
    register long r0 __asm__("x0") = (long)(fd);
    register long r1 __asm__("x1") = (long)buff;
    register long r2 __asm__("x2") = count;
    register long r3 __asm__("x3") = (long)&ret;

    asm volatile(
        "svc #0\n"
        : "=x"(r0)
        : "x"(r7), "0"(r0), "x"(r1), "x"(r2), "x"(r3)
        : "memory");

    if (r0 != E_NOERR) {
        ret = -r0;
    }

    return ret;
}

int read(FILE *fd, void *buff, unsigned int count)
{
    int ret = 0;
    register long r7 __asm__("x7") = SYS_FREAD;
    register long r0 __asm__("x0") = (long)(fd);
    register long r1 __asm__("x1") = (long)buff;
    register long r2 __asm__("x2") = count;
    register long r3 __asm__("x3") = (long)&ret;

    asm volatile(
        "svc #0\n"
        : "=x"(r0)
        : "x"(r7), "0"(r0), "x"(r1), "x"(r2), "x"(r3)
        : "memory");

    if (r0 != E_NOERR) {
        ret = -r0;
    }

    return ret;

}

FILE *open(const char *filename, uint8_t mode)
{
    FILE *ret = NULL;

    register long r7 __asm__("x7") = SYS_FOPEN;
	register long r0 __asm__("x0") = (long)ret;
	register long r1 __asm__("x1") = (long)filename;
	register long r2 __asm__("x2") = mode;

	asm volatile(
		"svc #0\n"
		: "=x"(r0)
		: "x"(r7), "0"(r0), "x"(r1), "x"(r2)
		: "memory");

    return ret;
}

err_t close(FILE *fd)
{
    register long r7 __asm__("x7") = SYS_FCLOSE;
	register long r0 __asm__("x0") = (long)fd;

	asm volatile(
		"svc #0\n"
		: "=x"(r0)
		: "x"(r7), "0"(r0)
		: "memory");

	return r0;
}

void *malloc(unsigned int n)
{
    void *ptr = NULL;
    register long r7 __asm__("x7") = SYS_MALLOC;
	register long r0 __asm__("x0") = (long)ptr;
    register long r1 __asm__("x1") = n;

    asm volatile(
        "svc #0\n"
        : "=x"(r0)
        : "x"(r7), "0"(r0), "x"(r1)
        : "memory");

    return ptr;
}

void free(void *p)
{
    register long r7 __asm__("x7") = SYS_FREE;
	register long r0 __asm__("x0") = (long)p;

    asm volatile(
        "svc #0\n"
        : "=x"(r0)
        : "x"(r7), "0"(r0)
        : "memory");
}

int time(void)
{
    int ticks = 0;
    register long r7 __asm__("x7") = SYS_TIME;
    register long r0 __asm__("x0") = (long)&ticks;

    asm volatile(
        "svc #0\n"
        : "=x"(r0)
        : "x"(r7), "0"(r0)
        : "memory");

    return ticks;
}