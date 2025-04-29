#include <stdlib.h>
#include <perf/perf.h>

#define SHUTDOWN_CODE 0x12345

int putchar(char c) {
    int ret = 0;
    char buffer[1];

    buffer[0] = c;

    ret = uart_write(buffer, 1);

    return ret;
}

int shutdown(char *buf)
{
    return SHUTDOWN_CODE;
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
    } /*else if (!strncmp("edit", buf, 4)) {
        return &call_editor;
    } */else if (!strncmp("listperf", buf, 8)) {
        return &perf_list;
    } else if (!strncmp("ls", buf, 2)) {
        return &ls;
    } else if (!strncmp("cat", buf, 3)) {
        return &cat;
    } else if (!strncmp("shutdown", buf, 8)) {
        return &shutdown;
    } else if (!strncmp("usage", buf, 5)) {
        return &usage;
    } else if (!strncmp("memcpy_test", buf, 11)) {
        return &memcpy_test;
    } else if (!strncmp("memset_test", buf, 11)) {
        return &memset_test;
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
            ret = func(buf+5);
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
				ret = run(buf, buflen);
                if (ret == SHUTDOWN_CODE) {
                    goto exit;
                }

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
