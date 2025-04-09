#include <stdlib/serial.h>
#include <stdlib/interrupts.h>
#include <drivers/gic400.h>
#include <memory/malloc.h>
#include <memory/hardware_reserve.h>
#include <drivers/sd.h>
#include <drivers/mailbox.h>
#include <memory/mmu.h>
#include <fs/ext2/ext2.h>
#include <fs/ext2/file.h>
#include <drivers/graphics/framebuffer.h>
#include "bootscreen.h"
#include <fs/ext2/part.h>
#include <trace/strace.h>
#include <stdlib/string.h>
#include <stdlib/shell.h>
#include <perf/perf.h>

struct block_device dev;

static inline unsigned int get_current_el(void)
{
    uint64_t v;
    asm volatile ("mrs %0, CurrentEl" : "=r" (v));
    return (unsigned int)(((v) >> 0x2U) & 0x3U);
}

void draw_example(struct framebuffer *fb);

void main()
{
    __depth = 0;
    push_trace("void main(void)","main",0,0,0,0);

    __info.x0 = 4;
    __info.y0 = 44;
    __info.x = 4;
    __info.y = 44;
    __info.color = 0x919CA6; 

    uart_init();
    printk("\x1b[1;32mUART FINISHED\x1b[1;0m\n");

    gic400_init((void *)0xFF840000UL);
    init_vectors();
    interrupt_barrier();
    enable_interrupts();
    printk("\x1b[1;32mINTERRUPTS FINISHED\x1b[1;0m\n");

    get_arm_address();
    printk("\x1b[1;32mFINISHED MAILBOX\x1b[1;0m\n");
    printk("ARM BASE ADDRESS: %8x\n", arm_base_address);
    printk("ARM_SIZE:         %x\n", arm_size);
    MEM_OFFSET = arm_base_address;

    init_framebuffer(0, 0, 32);
    printk("\x1b[1;32mFRAMEBUFFER INITIALIZED\x1b[1;0m\n");
    printk("\taddr: %8x size: %d\n", fb.buf, fb.buf_size);

    identity_map();

    enable_MMU();

    memory_init(0xFA000);
    printk("\x1b[1;32mMEMORY FINISHED\x1b[1;0m\n");
    global_heap = create(ALIGN, ALIGN*64);
    printk("\x1b[1;32mHEAP FINISHED WITH %d REGIONS\x1b[1;0m\n", global_heap->n_regions);

    timer_init();
    printk("\x1b[1;32mTIMER FINISHED\x1b[1;0m\n");

    bcm_2708_get_state();    
    
    disable_interrupts();

    draw_rect(0,32,647,1079,0xB52020,1);
    struct chr_dat tmp = __info;
    tmp.y0 -= 8;
    tmp.y -= 8;
    print_screen(&tmp, "FUNCTION HISTORY");
   
    sd_init();
    printk("\x1b[1;32mSD FINISHED\x1b[1;0m\n");

    init_events();
    printk("\x1b[1;32mPERF MAP FINISHED\x1b[1;0m\n");

    uint32_t super_sector = print_MBR_table();

    initial_fs(&emmc_dev->bd);
    printk(GREEN("EXT2: Created fs\n"));

    assign_superblock(super_sector);
    printk(GREEN("EXT2: Assigned Superblock\n"));

    read_BGD();
    printk(GREEN("EXT2: Read BGD table\n"));

    populate_fs();
    printk(GREEN("EXT2: Prepared root\n"));

//    enable_interrupts();

    printk(GREEN("EXT2: INITIALIZED\n"));

    printk("Current Exception Level: %d\n", get_current_el());

    printk("\nWaiting for serial port to be ready (press any key)\n");
    uart_getc();

	printk("\x1b[2J");
	printk(bootmsg);

    /* Enter our "shell" */
    ret = shell();
    if (ret == 0) {
        printk(RED("KERNEL FAILED\n"));
        while(1);
    } else {
        // Shutdown process
        printk(GREEN("SHUTTING DOWN\n"));
        disable_interrupts();
        sd_reset();
        printk(GREEN("SD RESET TO IDLE\n"));
        sd_power_off();
        printk(GREEN("SD POWERED OFF\n"));
        while(1);
    }
}
