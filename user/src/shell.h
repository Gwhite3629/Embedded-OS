#ifndef _SHELL_H_
#define _SHELL_H_

#include "../include/types.h"

typedef (int)(* command_t)(const char *buf);

uint32_t shell(void);
command_t *interpret(const char *buf);
int run(const char *buf);

#endif // _SHELL_H_