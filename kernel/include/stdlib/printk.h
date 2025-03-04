#ifndef _PRINTK_H_
#define _PRINTK_H_

#include "../drivers/graphics/framebuffer.h"
#include "serial.h"
#include "types.h"
#include "err.h"

#define RESET       "\x1b[22m"

#define BOLD(s)     ("\x1b[1m" s RESET)
#define DIM(s)      ("\x1b[2m" s RESET)
#define BLINK(s)    ("\x1b[5m" s RESET)

#define RED(s)      "\x1b[31m" s "\x1b[0m"
#define GREEN(s)    "\x1b[32m" s "\x1b[0m"
#define YELLOW(s)   "\x1b[33m" s "\x1b[0m"
#define BLUE(s)     "\x1b[34m" s "\x1b[0m"
#define MAGENTA(s)  "\x1b[35m" s "\x1b[0m"
#define CYAN(s)     "\x1b[36m" s "\x1b[0m"

int printk(char *fmt, ...);
int print_screen(struct chr_dat *dat, char *format, ...);
/*
int printk(char *string,...);
int print_screen(struct chr_dat dat, char *string,...);
*/

#endif // _PRINTK_H_
