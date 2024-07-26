#ifndef _QSORT_H_
#define _QSORT_H_

#include "types.h"

#define qsort(arg, tot, size, cmp) \
    quicksort(arg, tot, size, cmp, arg)

void quicksort (
    void *const pbase,
    size_t total_elems,
    size_t size,
	__compar_fn_t cmp,
    void *arg);

#endif // _QSORT_H_