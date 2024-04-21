#ifndef _SERIAL_H_
#define _SERIAL_H_

#include "../stdlib.h"

void uart_init(void);
void uart_putc(unsigned char byte);
unsigned char uart_getc(void);
int32_t uart_getc_noblock(void);
uint32_t uart_write(const unsigned char *buffer, size_t size);

#endif // _SERIAL_H_