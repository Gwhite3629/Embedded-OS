#include <stdlib/serial.h>
#include <stdlib/interrupts.h>
#include <drivers/gic400.h>
#include <memory/malloc.h>
#include <memory/hardware_reserve.h>
#include <drivers/sd.h>
#include <editor/editor.h>
#include <perf/perf.h>
#include <drivers/mailbox.h>
#include <memory/mmu.h>
#include <fs/ext2/ext2.h>
#include <fs/ext2/file.h>
#include <drivers/graphics/framebuffer.h>
#include "bootscreen.h"
#include <fs/ext2/part.h>
#include <trace/strace.h>
#include <stdlib/string.h>

typedef int (*command_t) (char *);

struct block_device dev;

static inline unsigned int get_current_el(void)
{
    uint64_t v;
    asm volatile ("mrs %0, CurrentEl" : "=r" (v));
    return (unsigned int)(((v) >> 0x2U) & 0x3U);
}

uint32_t shell(void);

void draw_example(struct framebuffer *fb);

int ls(char *buf)
{
    int ret = E_NOERR;
    char c = 0;
    char *spath = NULL;
    fs_tree *dir = NULL;
    printk(YELLOW("BUF: \"%s\", len: %d\n"), buf, strlen(buf));
    printk(YELLOW("PWD: \"%s\", len: %d\n"), fs->pwd, strlen(fs->pwd));
    if (strlen(buf) < 4) {
        new(spath, strlen(fs->pwd) + 1, char);
        memset(spath, 0, strlen(fs->pwd) + 1);
        memcpy(spath, fs->pwd, strlen(fs->pwd));
    } else {
        new(spath, strlen(buf + 3) + 1, char);
        memset(spath, 0, strlen(buf + 3) + 1);
        memcpy(spath, buf + 3, strlen(buf + 3));
    }

    printk("Raw ls path: %s\n", spath);
    int j = 0;
    while (c = spath[j]) {
        printk("%x ", c);
        j++;
    }
    printk("\n");

    if ((strlen(spath) == 1) & (spath[0] == '/')) {
        dir = root;
    } else {
        struct entry_info res = traverse_fs(spath);
        ret = res.ret;
        if ((ret != E_NOERR) | (res.type != FS_DIR)) {
            printk(RED("ls: invalid path\n"));
            goto exit;
        }
        dir = res.entry->dir;
    }
   
    printk("n entries: %d\n", dir->n_entries);

    for (int i = 0; i < dir->n_entries; i++) {
        printk("%1x ", dir->type[i]);
        switch (dir->type[i]) {
            case FS_FILE:
                printk(DIM("%16s %8d\n"), dir->entries[i]->file->name, dir->entries[i]->file->size);
                break;
            case FS_DIR:
                printk(BLUE("%16s %8d\n"), dir->entries[i]->dir->name, dir->entries[i]->dir->n_entries);
                break;
            default:
                printk(DIM(YELLOW("UNKNOWN TYPE\n")));
                break;
        }
    }

exit:
    if (spath) {
        del(spath);
    }
    return ret;
}

int echo(char *buf)
{
    printk("\n%s\n", buf+5);
    return 0;
}

int clear(char *buf)
{
    printk("\x1b[2J]");
    return 0;
}

int show_time(char *buf)
{
    printk("\n%d\n", tick_counter);
    return 0;
}

int call_editor(char *buf)
{
    int ret = E_NOERR;
    char c = 0;
    char *spath = NULL;
    printk(YELLOW("BUF: \"%s\", len: %d\n"), buf, strlen(buf));
    printk(YELLOW("PWD: \"%s\", len: %d\n"), fs->pwd, strlen(fs->pwd));
    if (strlen(buf) < 6) {
        new(spath, strlen(fs->pwd) + 1, char);
        memset(spath, 0, strlen(fs->pwd) + 1);
        memcpy(spath, fs->pwd, strlen(fs->pwd));
    } else {
        new(spath, strlen(buf + 5) + 1, char);
        memset(spath, 0, strlen(buf + 5) + 1);
        memcpy(spath, buf + 5, strlen(buf + 5));
    }

    printk("Raw edit path: %s, %d\n", spath, strlen(spath));
    int j = 0;
    while (c = spath[j]) {
        printk("%x ", c);
        j++;
    }
    printk("\n");

    ret = editor(spath);

exit:
    if (spath) {
        del(spath);
    }

    return ret;
}

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

//    fs = ext2_init(&emmc_dev->bd);

    initial_fs(&emmc_dev->bd);
    printk(GREEN("EXT2: Created fs\n"));

    assign_superblock(super_sector);
    printk(GREEN("EXT2: Assigned Superblock\n"));

    read_BGD();
    printk(GREEN("EXT2: Read BGD table\n"));

    populate_fs();
    printk(GREEN("EXT2: Prepared root\n"));

    printk(GREEN("EXT2: INITIALIZED\n"));

//    enable_interrupts();

    printk("Current Exception Level: %d\n", get_current_el());

    printk("\nWaiting for serial port to be ready (press any key)\n");
    uart_getc();

	printk("\x1b[2J");
	printk(bootmsg);

    /* Enter our "shell" */
    shell();

    printk("KERNEL FAILED\n");
    while(1);
}

int putchar(char c) {
    int ret = 0;
    char buffer[1];

    buffer[0] = c;

    ret = uart_write(buffer, 1);

    return ret;
}

command_t interpret(char *buf, int buflen)
{
    // Print command
    if (!strncmp("print", buf, 5)) {
        return &echo;
    // Clear command
    } else if (!strncmp("clear", buf, 5)) {
        return &clear;
    } else if (!strncmp("time", buf, 4)) {
        return &show_time;
    } else if (!strncmp("edit", buf, 4)) {
        return &call_editor;
    } else if (!strncmp("listperf", buf, 8)) {
        return &perf_list;
    } else if (!strncmp("ls", buf, 2)) {
        return &ls;
    } else if (buflen == 0) {
        printk("\n");
        return NULL;
    } else {
        printk("\nUnknown Command\n");
        return NULL;
    }
}

int run(char *buf, int buflen)
{
    int ret = 0;
    command_t func = NULL;
    uint64_t overflow = 0;

    if (!strncmp("perf", buf, 4)) {
        func = interpret(buf+5, buflen-4);
        if (func != NULL) {
            begin_profiling();
            ret = func(buf);
            overflow = end_profiling();
            if (ret != 0) return ret;
            printk("\nOverflow: %d\n", overflow);
            perf_print();
            ret = perf_cleanup();
        }
    } else {
        printk("\n");
        func = interpret(buf, buflen);
        if (func != NULL) { 
            ret = func(buf);
        }
    }

    return ret;
}

uint32_t shell(void)
{
    int ret = 0;
    char ch;
	uint32_t buflen = 0;
	char *buf;

    new(buf, 4096, char);

    /* Enter our "shell" */

    printk("Entered shell\n");

    memset(buf, '\0', 4096);

	while (1) {
		switch(ch = uart_getc()) {
            // Backspace or delete
			case(0x8):
			case(0x7f):
                if (buflen > 0) {
				    buflen--;
                    buf[buflen] = '\0';
                }
				break;
            // Newline or carriage return
			case(0xa):
			case(0xd):
				run(buf, buflen);

                memset(buf, '\0', 4096);
                buflen = 0;
				break;
            // Any non-special character
			default:
                buf[buflen] = ch;
                buflen++;
                break;
        }
        // Display character to screen
        uart_putc(ch);
        if (ret != 0) {
            printk("\nERROR: %d\n", ret);
        }
	}
exit:
    return ret;
}
