#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_
#include "../../kernel/include/fs/ext2/fstypes.h"

#include "../../kernel/include/syscalls/syscalls.h"

int serial_write(const void *buff, unsigned int count);
unsigned char serial_read(void);
int write(FILE *fd, const void *buff, unsigned int count);
int read(FILE *fd, void *buff, unsigned int count);
FILE *open(const char *filename, uint8_t mode);
err_t close(FILE *fd);
void *malloc(unsigned int n);
void free(void *p);
int time(void);
#endif // _SYSCALLS_H_