#ifndef PATH_H
#define PATH_H

#include "commands.h"

typedef struct path_t {
    char name[32];
    int hash;
    char path[4096];
} path_t;

static inline int
path_cmp(const void *va, const void *vb)
{
    const path_t *a = (path_t *)va;
    const path_t *b = (path_t *)vb;
    return (int)(a->hash - b->hash);
}

static inline path_t
path_lookup(char *name, path_t *path_table, int size)
{
    path_t zero = {
        "None",
        0,
        "None"
    };
    int key = hash(name);
    path_t table = {"None",key,"None"};
    path_t *r = bsearch(&table, path_table, size, sizeof(path_t), &path_cmp);
    return r ? *r : zero;
}

#endif // PATH_H