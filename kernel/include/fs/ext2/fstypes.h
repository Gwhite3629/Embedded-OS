#ifndef _FSTYPES_H_
#define _FSTYPES_H_

#include "../../stdlib.h"
// #include "../../drivers/sd.h"

typedef struct superblock {
    uint32_t n_inodes;           // Total number of inodes in filesystem
    uint32_t n_blocks;           // Total number of blocks in filesystem
    uint32_t n_superuser;        // Number of blocks for superuser
    uint32_t n_unalloc_blocks;   // Total number of unallocated blocks
    uint32_t n_unalloc_inodes;   // Total number of unallocated inodes
    uint32_t block_number_super; // Block number of the superblock
    uint32_t block_shift;      // Number to shift 1024 left by to get blocksize
    uint32_t frag_shift;       // Number to shift 1024 left by to get fragsize
    uint32_t n_block_in_group; // Number of blocks in each block group
    uint32_t n_frag_in_group;  // Number of fragments in each block group
    uint32_t n_inode_in_group; // Number of inodes in each block group
    uint32_t last_mount_time;  // Time of last mount (POSIX)
    uint32_t last_write_time;  // Time of last write (POSIX)
    uint16_t n_mount_nocheck;  // Number of times volume has been mounted since
                               // consistency check
    uint16_t n_mount_nocheck_max; // Number of mounts allowed before check
    uint16_t signature;           // 0xef53 EXT2 signature
    uint16_t FS_state;            // State of filesystem
    uint16_t error_action;        // What to do on error
    uint16_t minor_ver;           // Minor portion of volume version
    uint32_t last_check_time;     // Time of last check (POSIX)
    uint32_t check_interval;      // Check Interval
    uint32_t OS_ID;               // ID of OS in which volume was created
    uint32_t major_ver;           // Major portion of volume version
    uint16_t res_user_ID;         // User ID that can use reserved blocks
    uint16_t res_group_ID;        // Group ID that can use reserved blocks
    uint32_t first_non_res_inode; // First non-reserved inode in filesystem
    uint16_t inode_size;          // Size of inode struct in bytes
    uint16_t host_block_group;    // Block group that this super is part of (for
                                  // backups)
    uint32_t opt_featutes;        // Optional features which are present
    uint32_t req_features;        // Required features which are present
    uint32_t rd_only_features; // Features that if not present require read only
    char blkid_FS_ID[16];      // Filesystem ID
    char vol_name[16];         // Volume name
    char last_mnt_path[64];    // Path volume was last mounted to
    uint32_t compression_type; // Compression algorithm used
    uint8_t n_blocks_files;    // Number of blocks to preallocate for files
    uint8_t n_blocks_dir; // Number of blocks to preallocate for directories
    uint16_t unused;
    char journal_ID[16]; // Journal ID
    uint32_t journal_inode;
    uint32_t journal_device;
    uint32_t head_orphan_inode_list; // Head of orphan inode list
} __attribute__((__packed__, __aligned__(1))) superblock_t;

// Rest of EXT2 superblock is unused

typedef struct block_group_descriptor {
    uint32_t block_bitmap_addr;
    uint32_t inode_bitmap_addr;
    uint32_t inode_table_addr;
    uint16_t n_unalloc_blocks;
    uint16_t n_unalloc_inodes;
    uint16_t n_dir;
    uint16_t pad1;
    uint8_t bg_reserved[12];
} __attribute__((__packed__)) block_group_descriptor_t; // 32 bytes

typedef struct {
    struct block_device* dev;
    superblock_t* superblock;
    block_group_descriptor_t* BGD;
    uint32_t start_block;
    uint32_t block_size;
    uint32_t blocks_per_group;
    uint32_t inodes_per_group;
    uint32_t n_groups;
    uint32_t bgd_blocks;
    char* pwd;
} EXT2_t;

typedef struct FILE {
    char name[256];
    uint32_t hash;
    uint32_t inode;
    uint32_t size;
    uint32_t create_time;
    uint32_t access_time;
    uint32_t modify_time;
    uint32_t flags;
    uint32_t nlink;

    EXT2_t* fs;

    char* file_buffer;

    uint32_t cur_offset;
    int refcount;
} FILE;

typedef struct inode_base {
    uint16_t inode_type_perm;  // Type and permissions
    uint16_t user_ID;          // User ID
    int32_t size_lower;        // Lower 32 bits of size in bytes
    uint32_t last_access_time; // Time of last access (POSIX)
    uint32_t creation_time;    // Time of creation (POSIX)
    uint32_t last_mod_time;    // Time of last modification (POSIX)
    uint32_t deletion_time;    // Time of deletion (POSIX)
    uint16_t group_ID;         // Group ID
    uint16_t n_hard_links;     // Count of hard links in inode (directories)
    uint32_t n_sectors;        // Count of sectors in use by inode
    uint32_t flags;            // Inode flags
    uint32_t OS_Val_1;         // OS Specific value 1
    uint32_t blocks[15];       // Blocks, last 3 are indirect
    uint32_t gen_num;          // Generation number
    uint32_t ext_attr_block;   // Extended attribute block (if ver >= 1)
    uint32_t f_size_dir_acl;   // top bits of file size or dir ACL (if ver >= 1)
    uint32_t frag_block_addr;  // Block address of fragment
    uint8_t OS_Val_2[12];      // OS Specific value 2
} __attribute__((__packed__, __aligned__(1))) inode_base_t; // 128 Bytes

typedef struct dirent {
    uint32_t inode_number;
    uint16_t size;
    uint8_t name_len;
    uint8_t type;
    char* name;
} __attribute__((__packed__, __aligned__(1))) dirent_t;

typedef union fs_entry {
    struct fs_tree* dir;
    FILE* file;
} fs_entry;

typedef struct fs_tree {
    char* name;
    uint32_t hash;
    uint32_t inode;
    uint32_t n_entries;
    uint8_t* type;
    fs_entry** entries;
} fs_tree;

struct entry_info {
    fs_entry* entry;
    uint8_t type;
    int ret;
};

#endif // _FSTYPES_H_
