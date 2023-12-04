#ifndef EVAL_H
#define EVAL_H

#include <stdatomic.h>

#include "help.h"
#include "path.h"
#include "config.h"

int eval(char **tokens, int n_tokens, atomic_bool *quit, path_t **path_table, int *size, config_t *cfg);

#endif // EVAL_H
