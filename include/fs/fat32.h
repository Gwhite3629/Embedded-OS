#ifndef _FAT32_H_
#define _FAT32_H_

#include "../stdlib.h"

#define __packed __attribute__((packed))

#define SECTOR_SIZE 512

typedef struct Bios_Parameter_Block {
    uint8_t BS_jmpBoot[3];      // Jump instruction to boot code
    char BS_OEMName[8];         // OEM Name Identifier
    uint16_t BPB_BytsPerSec;    // N Bytes per Sector
    uint8_t BPB_SecPerClus;     // N Sectors per Cluster
    uint16_t BPB_RsvdSecCnt;    // Number of reserved sectors at start of Volume
    uint8_t BPB_NumFATs;        // Number of FATs
    uint16_t BPB_RootEntCnt;    // 0 for FAT32
    uint16_t BPB_TotSec16;      // 0 for FAT32
    uint8_t BPB_Media;          // 0xF0 for removable media, 0xF8 for fixed
    uint16_t BPB_FATSz16;       // 0 for FAT32
    uint16_t BPB_SecPerTrk;     // sectors per track, irrelevant
    uint16_t BPB_NumHeads;      // number of heads, irrelevant
    uint32_t BPB_HiddSec;       // number of hidden sectors on partition before FAT
    uint32_t BPB_TotSec32;      // total number of sectors per volume
    uint32_t BPB_FATSz32;       // number of sectors for one FAT
    uint16_t BPB_ExtFlags;      // flag bits
    uint16_t BPB_FSVer;         // Hi: major, lo: minor revision #
    uint32_t BPB_RootClus;      // cluster number of root cluster
    uint16_t BPB_FSInfo;        // sector number of FSINFO struct
    uint16_t BPB_BkBootSec;     // 0
    uint8_t BPB_Reserved[12];   // Reserved, 0
    uint8_t BS_DrvNum;          // Irrelevant
    uint8_t BS_Reserved1;       // Reserved, 0
    uint8_t BS_BootSig;         // 0x29
    uint32_t BS_VolID;          // serial number
    uint8_t BS_VolLab[11];      // volume label
    char BS_FilSysType[8];      // "FAT32"
    uint8_t RSRVD[420];         // Reserved, 0
    uint16_t Signature_Word;    // Hi: 0xAA, lo: 0x55
} BPB __packed;

typedef struct FAT32_FS {
    uint8_t is_mounted;
    uint8_t status; // 0: Not Contiguous 2: Contiguous 3: Frag
    uint8_t attr;   // Attributes;
    uint8_t n_fats; // 1 or 2
    uint8_t dflag; // Access dirty flag
    uint8_t fsiflag; // FSInfo dirty flag
    uint16_t csize; // Cluster size
    uint32_t lac; // Last allocated cluster
    uint32_t nfc; // Number of free clusters
    uint32_t n_entries;
    uint16_t n_rootdir;
    uint32_t fsize; // Number of secters/entry
    uint32_t vbase; // Volume base sector
    uint32_t fbase; // FAT base sector
    uint32_t cbase; // Root directory cluster
    uint32_t dbase; // Data base sector
    uint32_t sclust; // Object data start cluster
    uint32_t current_sector; // Sector in current
    uint8_t current_access[SECTOR_SIZE]; // Current access
} FS;

typedef struct {
    FS *fs;
    uint8_t flags;
    err_t err;
    uint32_t fptr;
    uint32_t cluster;
    uint16_t sector;
    uint16_t dir_sector;
    uint8_t *dir_ptr;
} FILE;

typedef struct {
    FS *fs;
    uint32_t dptr;
    uint32_t cluster;
    uint16_t sector;
    uint8_t *dir;
    uint8_t fn[12];
} DIR;

typedef struct {
    uint32_t fsize;
    uint16_t fdate;
    uint16_t ftime;
    uint8_t fattrib;
    char fname[13];
} FILEINFO;

typedef uint32_t FAT32_ENTRY;

// FAT32 entry values
#define SET_FREE(ENTRY) ((ENTRY) |= 0x0000000)
#define SET_EOF(ENTRY) ((ENTRY) |= 0xFFFFFFF)
#define SET_BAD(ENTRY) ((ENTRY) |= 0xFFFFFF7)

uint32_t get_entry(FS *fs, uint32_t n);
err_t set_entry(FS *fs, uint32_t n, uint32_t v);
err_t remove_chain(FS *fs, uint32_t n, uint32_t prev);
uint32_t create_chain(FS *fs, uint32_t n);

err_t dir_clear(FS *fs, uint32_t n);
err_t dir_set_idx(DIR *dir, uint32_t offset);
err_t dir_next(DIR *dir, int stretch);
err_t dir_alloc(DIR *dir, uint32_t n_entries);
err_t dir_read(DIR *dir, int sel);
err_t dir_find(DIR *dir);
err_t dir_register(DIR *dir);
err_t dir_remove(DIR *dir);
void dir_getfileinfo(DIR *dir, FILEINFO *finfo);
err_t dir_create_name(DIR *dir, const char **path);
err_t dir_follow_path(DIR *dir, const char *path);

int get_ldnumber(const char **path);
uint32_t check_fs(FS *fs, uint16_t sector);
uint32_t find_volume(FS *fs, uint32_t partition);
err_t mount_volume(const char **path, FS **rfs, uint8_t mode);
err_t validate(FS *fs, FS **rfs);

#define MAX_DIR         0x200000
#define MAX_CLUSTERS    0x0FFFFFF5
#define DIR_ENTRY_SIZE  32
#define DIR_DEL_M       0xE5

// Directory offsets
#define DIR_Name        0
#define DIR_Attr        11
#define DIR_NTres       12
#define DIR_CrtTime10   13
#define DIR_CrtTime     14
#define DIR_LstAccDate  18
#define DIR_FstClusHi   20
#define DIR_ModTime     22
#define DIR_FstClusLo   26
#define DIR_FileSize    28

#define A_VOL       0x08
#define A_MASK      0x3F
#define A_RDONLY    0x01
#define A_HIDDEN    0x02
#define A_SYSTEM    0x04
#define A_DIR       0x10
#define A_ARCHIVE   0x20

#define	FA_READ				0x01
#define	FA_WRITE			0x02
#define	FA_OPEN_EXISTING	0x00
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define	FA_OPEN_APPEND		0x30

// Character code macros
#define IsUpper(c)		((c) >= 'A' && (c) <= 'Z')
#define IsLower(c)		((c) >= 'a' && (c) <= 'z')
#define IsDigit(c)		((c) >= '0' && (c) <= '9')
#define IsSeparator(c)	((c) == '/' || (c) == '\\')
#define IsTerminator(c)	((c) < ('!'))

#endif // _FAT32_H_
