#include <fs/ext2/part.h>
#include <drivers/sd.h>
#include <memory/malloc.h>
#include <stdlib.h>

int print_MBR_table(void)
{
    int r = E_NOERR;
    uint8_t *sector_data = NULL;
    
    uint32_t super_sector;

    struct MBR current_part;

    new(sector_data, 512, uint8_t);

    r = sd_read(sector_data, 1, 0);
    if (r != 1) {
        printk(RED("EXT2: Failed to read sector: 0\n"));
        return r;
    }

    for (int i = 0; i < 4; i++) {
        current_part = *((struct MBR *)&sector_data[446 + 16*i]);
        printk(GREEN("\n------ BEGIN MBR %1d ------\n"), i);
        printk(CYAN(   "Bootable               %2x\n"), current_part.bootable);                                                        
        printk(CYAN(   "Starting Head          %2x\n"), current_part.start_head);                                                      
        printk(CYAN(   "Starting Sector        %2x\n"), current_part.start_sect);                                                      
        printk(CYAN(   "Starting Cylinder     %3x\n"), \
        (((current_part.start_cyl_hi & 0x3) << 8) | current_part.start_cyl) & 0x3ff);                                                        
        printk(CYAN(   "System ID              %2x\n"), current_part.system_id);                                                       
        printk(CYAN(   "Ending Head            %2x\n"), current_part.end_head);                                                        
        printk(CYAN(   "Ending Sector          %2x\n"), current_part.end_sect);                                                        
        printk(CYAN(   "Ending Cylinder       %3x\n"), \
        (((current_part.end_cyl_hi & 0x3) << 8) | current_part.end_cyl) & 0x3ff);                                                          
        printk(CYAN(   "Relative Sector  %8x\n"), current_part.relative_sector);                                                       
        printk(CYAN(   "Total Sectors    %8x\n"), current_part.partition_sectors);                                                     
        printk(GREEN(  "-------- END MBR --------\n"));
        if (i == 1) {
            super_sector = current_part.relative_sector;
        }
    }
    return super_sector;
exit:
    return ret;
}
