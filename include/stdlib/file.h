#ifndef _FILE_H_
#define _FILE_H_

#include "../stdlib.h"
#include "../fs/fat32.h"

err_t f_open(FILE *fp, const char *path, uint8_t mode);
err_t f_close(FILE *fp);
err_t f_read(void *buffer, size_t size, size_t count, FILE *fp);
err_t f_write(const void *buffer, size_t size, size_t count, FILE *fp);
err_t f_seek(FILE *fp, long offset, int origin);

#endif // _FILE_H_