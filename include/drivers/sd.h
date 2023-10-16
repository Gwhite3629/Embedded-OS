#ifndef _SD_H_
#define _SD_H_

#include "../stdlib/types.h"

#define CTRL_SYNC 0
#define GET_SECTOR_COUNT 1
#define GET_SECTOR_SIZE 2
#define GET_BLOCK_SIZE 3
#define CTRL_TRIM 4

int sd_init(void);
int sd_status(void);
int sd_read(char *buff, uint16_t sector, size_t n);
int sd_write(const char *buff, uint16_t sector, size_t n);
int sd_ioctl(uint8_t cmd, void *buff);

#endif // _SD_H_