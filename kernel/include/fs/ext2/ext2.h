#ifndef _EXT2_H_
#define _EXT2_H_

#include "../../stdlib.h"

#include "../../drivers/sd.h"

#include "fstypes.h"

extern EXT2_t *fs;

#define SUPERBLOCK_SIZE     1024

// States
#define FS_STATE_CLEAR      0
#define FS_STATE_ERROR      1

// Error handling methods
#define FS_ERROR_IGNORE     1
#define FS_ERROR_REMOUNT    2
#define FS_ERROR_PANIC      3

// OS IDs
#define FS_ID_LINUX         
#define FS_ID_GNU_HURD      1
#define FS_ID_MASIX         2
#define FS_ID_FREEBSD       3
#define FS_ID_OTHER         4

uint32_t ext2_get_file_size(FILE *file);

void ext2_mkdir(FILE *file, char *name, uint16_t perm);
void ext2_mkfile(FILE *file, char *name, uint16_t perm);

void ext2_unlink(FILE *file, char *name);
char **ext2_listdir(FILE *file);
FILE *ext2_finddir(FILE *file, char *name);

void ext2_create_entry(FILE *file, char *name, uint32_t inode);
void ext2_remove_entry(FILE *file, char *name);

void ext2_chmod(FILE *name, uint32_t mode);

uint32_t ext2_read(FILE *file, uint32_t offset, uint32_t size, char *buf);
uint32_t ext2_write(FILE *file, uint32_t offset, uint32_t size, char *buf);

void ext2_open(FILE *file, uint32_t flags);
void ext2_close();

void write_bgd(void);
void write_superblock(void);

uint32_t ext2_alloc_block(void);
void ext2_free_block(uint32_t block);

fs_tree *get_ext2_root(inode_base_t *inode);
void ext2_init(struct block_device *dev);

int initial_fs(struct block_device *dev);

int assign_superblock(uint32_t super_sector);

int read_BGD(void);

int populate_fs(void);

#endif // _EXT2_H_
