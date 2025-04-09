#ifndef _SHELL_H_
#define _SHELL_H_

typedef int (*command_t) (char *);

uint32_t shell(void);

#endif // _SHELL_H_
