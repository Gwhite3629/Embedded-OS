#ifndef _PART_H_
#define _PART_H_

#include "../../stdlib.h"

struct __attribute__((__packed__, aligned(8))) MBR {
    unsigned bootable:8;
    unsigned start_head:8;
    unsigned start_sect:6;
    unsigned start_cyl_hi:2;
    unsigned start_cyl:8;
    unsigned system_id:8;
    unsigned end_head:8;
    unsigned end_sect:6;
    unsigned end_cyl_hi:2;
    unsigned end_cyl:8;
    uint32_t relative_sector;
    uint32_t partition_sectors;
};

int print_MBR_table(void);
int print_GPT_table(void);

#endif // _PART_H_
