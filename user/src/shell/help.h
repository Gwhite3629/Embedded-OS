#ifndef HELP_H
#define HELP_H

#define MAX_LINE 4096

typedef enum eval_type {SCRIPT, PROGRAM, COMMAND} eval_type;

typedef struct eval_t {
    eval_type type;
} eval_t;

// Colors:
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define ncolors 8

#endif // HELP_H
