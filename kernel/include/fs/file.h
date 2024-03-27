#ifndef _FILE_H_
#define _FILE_H_

#include "../stdlib.h"
#include "fat32.h"

typedef struct {
    uint8_t fmt;
    uint8_t nfat;
    uint32_t align;
    uint32_t n_root;
    uint32_t au_size;
} MKFS_PARM;

err_t f_open (FILE* fp, const char* path, uint8_t mode);				/* Open or create a file */
err_t f_close (FILE* fp);											/* Close an open file object */
err_t f_read (FILE* fp, void* buff, unsigned int btr, unsigned int* br);			/* Read data from the file */
err_t f_write (FILE* fp, const void* buff, unsigned int btw, unsigned int* bw);	/* Write data to the file */
err_t f_lseek (FILE* fp, size_t ofs);								/* Move file pointer of the file object */
err_t f_truncate (FILE* fp);										/* Truncate the file */
err_t f_sync (FILE* fp);											/* Flush cached data of the writing file */
err_t f_opendir (DIR* dp, const char* path);						/* Open a directory */
err_t f_closedir (DIR* dp);										/* Close an open directory */
err_t f_readdir (DIR* dp, FILEINFO* fno);							/* Read a directory item */
err_t f_findfirst (DIR* dp, FILEINFO* fno, const char* path, const char* pattern);	/* Find first file */
err_t f_findnext (DIR* dp, FILEINFO* fno);							/* Find next file */
err_t f_mkdir (const char* path);								/* Create a sub directory */
err_t f_unlink (const char* path);								/* Delete an existing file or directory */
err_t f_rename (const char* path_old, const char* path_new);	/* Rename/Move a file or directory */
err_t f_stat (const char* path, FILEINFO* fno);					/* Get file status */
err_t f_chmod (const char* path, uint8_t attr, uint8_t mask);			/* Change attribute of a file/dir */
err_t f_utime (const char* path, const FILEINFO* fno);			/* Change timestamp of a file/dir */
err_t f_chdir (const char* path);								/* Change current directory */
err_t f_chdrive (const char* path);								/* Change current drive */
err_t f_getcwd (char* buff, unsigned int len);							/* Get current directory */
err_t f_getfree (const char* path, uint32_t* nclst, FS** fatfs);	/* Get number of free clusters on the drive */
err_t f_getlabel (const char* path, char* label, uint32_t* vsn);	/* Get volume label */
err_t f_setlabel (const char* label);							/* Set volume label */
err_t f_forward (FILE* fp, unsigned int(*func)(const uint8_t*,unsigned int), unsigned int btf, unsigned int* bf);	/* Forward data to the stream */
err_t f_expand (FILE* fp, size_t fsz, uint8_t opt);					/* Allocate a contiguous block to the file */
err_t f_mount (FS* fs, const char* path, uint8_t opt);			/* Mount/Unmount a logical drive */
err_t f_mkfs (const char* path, const MKFS_PARM* opt, void* work, unsigned int len);	/* Create a FAT volume */
err_t f_fdisk (uint8_t pdrv, const uint16_t ptbl[], void* work);		/* Divide a physical drive into some partitions */
err_t f_setcp (uint32_t cp);											/* Set current code page */
int f_putc (char c, FILE* fp);										/* Put a character to the file */
int f_puts (const char* str, FILE* cp);								/* Put a string to the file */
int f_printf (FILE* fp, const char* str, ...);						/* Put a formatted string to the file */
char* f_gets (char* buff, int len, FILE* fp);

#endif // _FILE_H_