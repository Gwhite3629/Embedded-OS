#ifndef _SHELL_H_
#define _SHELL_H_

#include "../include/types.h"

typedef int (* command_t) (char *);

uint32_t shell(void);
command_t interpret(char *buf);
int run(char *buf);

#endif // _SHELL_H_