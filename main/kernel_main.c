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
#include "bootscreen.h"

typedef int (*command_t) (const char *);

static inline unsigned int get_current_el(void)
{
    uint64_t v;
    asm volatile ("mrs %0, CurrentEl" : "=r" (v));
    return (unsigned int)(((v) >> 0x2U) & 0x3U);
}

uint32_t shell(void);

int echo(const char *buf)
{
    printk("\n%s\n", buf+5);
    return 0;
}

int clear(const char *buf)
{
    printk("\x1b[2J]");
    return 0;
}

int show_time(const char *buf)
{
    printk("\n%d\n", tick_counter);
    return 0;
}

int call_editor(const char *buf)
{
    return editor(buf+5);
}

void main()
{
    uart_init();
    printk("\x1b[1;32mUART FINISHED\x1b[1;0m\n");
    gic400_init((void *)0xFF840000UL);
    init_vectors();
    interrupt_barrier();
    enable_interrupts();
    printk("\x1b[1;32mINTERRUPTS FINISHED\x1b[1;0m\n");

    get_arm_address();
    printk("\x1b[1;32mFINISHED MAILBOX\x1b[1;0m\n");
    printk("ARM BASE ADDRESS: %x\n", arm_base_address);
    printk("ARM_SIZE:         %x\n", arm_size);
    MEM_OFFSET = arm_base_address;

    identity_map();

    enable_MMU();

    memory_init(0xFA000);
    printk("\x1b[1;32mMEMORY FINISHED\x1b[1;0m\n");
    global_heap = create(ALIGN, ALIGN*64);
    printk("\x1b[1;32mHEAP FINISHED WITH %d REGIONS\x1b[1;0m\n", global_heap->n_regions);

    struct block_device *dev;
    new(dev, 1, struct block_device);
    printk("\x1b[1;32mCREATED BLOCK DEVICE\x1b[1;0m\n");

    timer_init();
    printk("\x1b[1;32mTIMER FINISHED\x1b[1;0m\n");

    bcm_2708_get_state();

    sd_card_init(&dev);
    printk("\x1b[1;32mSD FINISHED\x1b[1;0m\n");

    init_events();
    printk("\x1b[1;32mPERF MAP FINISHED\x1b[1;0m\n");

    fs_init();
    printk("\x1b[1;32mFILESYSTEM INITIALIZED\x1b[1;0m\n");

    //ext2_init(dev);
    //printk("\x1b[1;32mEXT2 INITIALIZED\x1b[1;0m\n");

    printk("Current Exception Level: %d\n", get_current_el());

    printk("\nWaiting for serial port to be ready (press any key)\n");
    uart_getc();

	printk("\x1b[2J");
	printk(bootmsg);

    shell();
	/* Enter our "shell" */
exit:
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

command_t interpret(const char *buf, int buflen)
{
    // Print command
    if (!strncmp("print", buf, 5)) {
        return &echo;
    // Clear command
    } else if (!strncmp("clear", buf, 5)) {
        return &clear;
    // Anything our shell doesn't know
    } else if (!strncmp("time", buf, 4)) {
        return &show_time;
    } else if (!strncmp("edit", buf, 4)) {
        return &call_editor;
    } else if (!strncmp("listperf", buf, 8)) {
        return &perf_list;
    } else if (buflen == 0) {
        printk("\n");
        return NULL;
    } else {
        printk("\nUnknown Command\n");
        return NULL;
    }
}

int run(const char *buf, int buflen)
{
    int ret = 0;
    command_t func = NULL;
    uint64_t overflow = 0;

    if (!strncmp("perf", buf, 4)) {
        func = interpret(buf+5, buflen-4);
        begin_profiling();
        ret = func(buf);
        overflow = end_profiling();
        if (ret != 0) return ret;
        printk("\nOverflow: %d\n", overflow);
        perf_print();
        ret = perf_cleanup();
    } else {
        func = interpret(buf, buflen);
        ret = func(buf);
    }

    return ret;
}

uint32_t shell(void)
{
    int ret = 0;
    char ch;
	uint32_t buflen = 0;
	char buf[4096];

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
		}
        // Display character to screen
        uart_putc(ch);
        if (ret != 0) {
            printk("\nERROR: %d\n", ret);
        }
	}
}
