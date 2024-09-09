#ifndef _INODE_H_
#define _INODE_H_

#include "../../stdlib/types.h"
#include "fstypes.h"

#define INODE_SIZE 128

static inline uint32_t inode_get_group(uint32_t inode_address, superblock_t superblock)
{
    return (inode_address - 1) / superblock.n_inode_in_group;
}

static inline uint32_t inode_get_index(uint32_t inode_address, superblock_t superblock)
{
    return (inode_address - 1) % superblock.n_inode_in_group;
}

static inline uint32_t inode_get_block(uint32_t index, superblock_t superblock)
{
    return (index * INODE_SIZE) / (1024 << superblock.block_shift);
}

// Inode types (top 4 bits)
#define INODE_FIFO  0x1000
#define INODE_CHAR  0x2000
#define INODE_DIR   0x4000
#define INODE_BLOCK 0x6000
#define INODE_FILE  0x8000
#define INODE_SYM   0xA000
#define INODE_SOCK  0xC000

// Inode permissions (bottom 12 bits)
#define INODE_O_EX  0x001
#define INODE_O_WR  0x002
#define INODE_O_RD  0x004
#define INODE_G_EX  0x008
#define INODE_G_WR  0x010
#define INODE_G_RD  0x020
#define INODE_U_EX  0x040
#define INODE_U_WR  0x080
#define INODE_U_RD  0x100
#define INODE_STICK 0x200
#define INODE_G_ID  0x400
#define INODE_U_ID  0x800

// Inode flags
#define INODE_SEC_DEL       0x00000001
#define INODE_KEEP_COPY     0x00000002
#define INODE_FILE_COMP     0x00000004
#define INODE_SYNC_UPDATE   0x00000008
#define INODE_IMMUTABLE     0x00000010
#define INODE_APPEND_ONLY   0x00000020
#define INODE_NODUMP        0x00000040
#define INODE_NOTIME        0x00000080
#define INODE_HASH_DIR      0x00010000
#define INODE_AFS_DIR       0x00020000
#define INODE_JOURNAL       0x00040000

// Inode read steps
// 1. Read Superblock for:
//      a. size of blocks
//      b. number of blocks per group
//      c. number of inodes per group
//      d. starting block of first group (block descriptor table)
// 2. Determine block group that inode belongs to
// 3. Read block group descriptor for that block group
// 4. Extract location of block group inode table from descriptor
// 5. Determine index of inode from inode table
// 6. Index inode table
// 7. Dir and File data are in data blocks from inode

void read_inode_metadata(EXT2_t *fs, inode_base_t *inode, uint32_t index);
void write_inode_metadata(EXT2_t *fs, inode_base_t *inode, uint32_t index);

uint32_t read_inode_filedata(EXT2_t *fs, inode_base_t *inode, uint32_t offset, uint32_t size, char *buf);
void write_inode_filedata(EXT2_t *fs, inode_base_t *inode, uint32_t index, uint32_t offset, uint32_t size, char *buf);

char *read_inode_block(EXT2_t *fs, inode_base_t *inode, uint32_t block);
void write_inode_block(EXT2_t *fs, inode_base_t *inode, uint32_t block, char *buf);

void read_disk_block(EXT2_t *fs, uint32_t block, char *buf);
void write_disk_block(EXT2_t *fs, uint32_t block, char *buf);

int alloc_inode_metadata_block(uint32_t *block_ptr, EXT2_t *fs, inode_base_t *inode, uint32_t index, char *buf, uint32_t block_overwrite);

uint32_t get_disk_block_number(EXT2_t *fs, inode_base_t *inode, uint32_t inode_block);
void set_disk_block_number(EXT2_t *fs, inode_base_t *inode, uint32_t index, uint32_t inode_block, uint32_t disk_block);

void alloc_inode_block(EXT2_t *fs, inode_base_t *inode, uint32_t index, uint32_t block);
void free_inode_block(EXT2_t *fs, inode_base_t *inode, uint32_t index, uint32_t block);

uint32_t alloc_inode(EXT2_t *fs);
void free_inode(EXT2_t *fs, uint32_t inode);

#endif // _INODE_H_