#include <stdlib/serial.h>
#include <stdlib/interrupts.h>
#include <drivers/gic400.h>
#include <memory/malloc.h>
#include <memory/hardware_reserve.h>
#include <drivers/sd.h>
#include <editor/editor.h>
#include <perf/perf.h>
#include "bootscreen.h"

uint32_t shell(void);

int echo(const char *buf)
{
    printf("\n%s\n", buf+6);
    return 0;
}

int clear(const char *buf)
{
    printf("\x1b[2J]");
    return 0;
}

int show_time(const char *buf)
{
    printf("\n%d\n", tick_counter);
    return 0;
}

int call_editor(const char *buf)
{
    return editor(buf+5);
}

void main()
{
    uart_init();
    printk("UART FINISHED\n");
    gic400_init((void *)0xFF840000UL);
    init_vectors();
    interrupt_barrier();
    enable_interrupts();
    printk("INTERRUPTS FINISHED\n");

    printk("Some sizes:\n");
    printk("\tCHUNK_INFO_SIZE:  %x\n", sizeof(smart_ptr));
    printk("\tREGION_INFO_SIZE: %x\n", sizeof(region_t));
    printk("\tHEAP_INFO_SIZE:   %x\n", sizeof(heap_t));
    printk("\tREGION_ARR_SIZE:  %x\n", sizeof(region_t *));
    printk("\tCHUNK_ARR_SIZE:   %x\n", sizeof(smart_ptr *));
    printk("\tCHAR PTR SIZE:    %x\n", sizeof(char *));

    memory_init(0xFA0000);
    printk("MEMORY FINISHED\n");
    global_heap = create(ALIGN, ALIGN*16);
    printk("HEAP FINISHED\n");

    sd_init();
    printk("SD FINISHED\n");

    timer_init();
    printk("TIMER FINISHED\n");

    init_events();
    printk("PERF MAP FINISHED\n");

    printk("\nWaiting for serial port to be ready (press any key)\n");
    uart_getc();

	printk("\x1b[2J");
	printk(bootmsg);

    shell();
	/* Enter our "shell" */
    while(1);
}

int putchar(char c) {
    int ret = 0;
    char buffer[1];

    buffer[0] = c;

    ret = uart_write((uint32_t *)buffer, 1);

    return ret;
}

command_t *interpret(const char *buf, int buflen)
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
    } else if (!strncmp("perflist", buf, 8)) {
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
    command_t *func = NULL;
    uint64_t overflow = 0;

    if (!strncmp("perf", buf, 4)) {
        func = interpret(buf+5);
        begin_profiling();
        ret = func(buf);
        overflow = end_profiling();
        if (ret != 0) return ret;
        printf("\nOverflow: %d\n", overflow);
        perf_print();
        ret = perf_cleanup();
    } else {
        func = interpret(buf);
        ret = func(buf, buflen);
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
