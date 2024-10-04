//#include <stddef.h>
#include "shell.h"
//#include <memset.h>
#include <memset.h>
//#include <string.h>
#include <string.h>
#include <types.h>
#include <syscalls.h>
#include <printf.h>

int putchar(char c) {
    int ret = 0;
    char buffer[1];

    buffer[0] = c;

    ret = serial_write((uint32_t *)buffer, 1);

    return ret;
}

int echo(char *buf)
{
    printf("\n%s", buf);
    return 0;
}

int clear(char *buf)
{
    printf("\x1b[2J]");
    return 0;
}

int show_time(char *buf)
{
    printf("\n%d", time());
    return 0;
}

uint32_t shell(void)
{
    int ret = 0;
    char ch;
	uint32_t buflen = 0;
	char buf[4096];

    /* Enter our "shell" */

    printf("Entered shell\n");

    memset(buf, '\0', 4096);

	while (1) {
		switch(ch = serial_read()) {
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
				run(buf);

                memset(buf, '\0', 4096);
                buflen = 0;
				break;
            // Any non-special character
			default:
                buf[buflen] = ch;
                buflen++;
		}
        // Display character to screen
        ret = putchar(ch);
        if (ret != 0) {
            printf("\nERROR: %d", ret);
        }
	}
}

int run(char *buf)
{
    command_t func = NULL;
    if (!strncmp("perf", buf, 4)) {
        func = interpret(buf+5);

    } else {
        func = interpret(buf);
        return (*func)(buf);
    }
    return 0;
}

command_t interpret(char *buf)
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
    } else {
        printf("\nUnknown Command");
        return NULL;
    }
}