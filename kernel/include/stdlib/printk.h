#ifndef _PRINTK_H_
#define _PRINTK_H_

#include "../drivers/graphics/framebuffer.h"
#include "serial.h"
#include "types.h"
#include "err.h"

#define RED(s)      ("\x1b[1;31m" s "\x1b[1;0m")
#define GREEN(s)    ("\x1b[1;32m" s "\x1b[1;0m")
#define YELLOW(s)   ("\x1b[1;33m" s "\x1b[1;0m")
#define BLUE(s)     ("\x1b[1;34m" s "\x1b[1;0m")
#define MAGENTA(s)  ("\x1b[1;35m" s "\x1b[1;0m")
#define CYAN(s)     ("\x1b[1;36m" s "\x1b[1;0m")

int printk(char *fmt, ...);
int print_screen(struct chr_dat *dat, char *format, ...);
/*
int printk(char *string,...);
int print_screen(struct chr_dat dat, char *string,...);
*/

#endif // _PRINTK_H_