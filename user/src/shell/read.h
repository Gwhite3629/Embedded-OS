#ifndef READ_H
#define READ_H

#include "help.h"

int readl(char *line, int *n_tokens, char ***tokens);

int parse(char *line, int *n_tokens, char ***tokens);

#endif // READ_H