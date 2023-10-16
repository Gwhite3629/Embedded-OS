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
    uint8_t status; // 0: Not Contiguous 2: Contiguous 3: Frag
    uint8_t n_fats; // 1 or 2
    uint8_t dflag; // Access dirty flag
    uint8_t fsiflag; // FSInfo dirty flag
    uint16_t csize; // Cluster size
    uint32_t lac; // Last allocated cluster
    uint32_t nfc; // Number of free clusters
    uint32_t n_entries;
    uint32_t fsize; // Number of secters/entry
    uint32_t vbase; // Volume base sector
    uint32_t fbase; // FAT base sector
    uint32_t cbase; // Root directory cluster
    uint32_t dbase; // Data base sector
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

typedef uint32_t FAT32_ENTRY;

// FAT32 entry values
#define SET_FREE(ENTRY) ((ENTRY) |= 0x0000000)
#define SET_EOF(ENTRY) ((ENTRY) |= 0xFFFFFFF)
#define SET_BAD(ENTRY) ((ENTRY) |= 0xFFFFFF7)

uint32_t get_entry(FS *fs, uint32_t n);
err_t set_entry(FS *fs, uint32_t n, uint32_t v);
err_t remove_chain(FS *fs, uint32_t n, uint32_t prev);
uint32_t create_chain(FS *fs, uint32_t n);

#endif // _FAT32_H_