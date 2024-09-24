#ifndef _PRINTK_H_
#define _PRINTK_H_

#include "../drivers/graphics/framebuffer.h"

int printk(char *string,...);
int print_screen(uint32_t x, uint32_t y, unsigned char attrs, char *string,...);

#endif // _PRINTK_H_