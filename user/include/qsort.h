#include "types.h"

#define qsort(arg, tot, size, cmp) \
    quicksort(arg, tot, size, cmp, arg)

void quicksort (
    void *const pbase,
    size_t total_elems,
    size_t size,
	__compar_d_fn_t cmp,
    void *arg);