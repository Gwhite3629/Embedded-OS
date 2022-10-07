#ifndef _LOADER_H_
#define _LOADER_H_

#include "../stdlib/err.h"
#include "proc.h"
#include "status.h"

typedef void* file_t; // placeholder

err_t load_proc(file_t ex, proc_t *p, const char *argv[]);

err_t map(proc_t *p);

err_t exec(proc_t *p);

#endif // _LOADER_H_