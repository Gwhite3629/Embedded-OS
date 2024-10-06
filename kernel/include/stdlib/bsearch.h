#ifndef _BSEARCH_H_
#define _BSEARCH_H_

#include "err.h"
#include "types.h"

extern inline __attribute__((__gnu_inline__)) void *
bsearch (const void *__key, const void *__base, size_t __nmemb, size_t __size,
	 __compar_fn_t __compar);

#endif // _BSEARCH_H_