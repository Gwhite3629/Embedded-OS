#ifndef _PERF_H_
#define _PERF_H_

#include <stdlib/types.h>
#include <stdlib/extra.h>
#include <stdlib/qsort.h>
#include <memory/malloc.h>

typedef struct perf_event {
    char *name;
    uint16_t event_number;
    uint32_t hash;
} perf_event_t;

static uint32_t perf_hash(char *string)
{
    unsigned long val = 5381;
    int c;
    while ((c = *string++)) {
        val = ((val << 5) + val) + c;
    }
    return (uint32_t)(val % (__INT_MAX__));
}

static int perf_cmp(const void *va, const void *vb)
{
    const perf_event_t *a = (const perf_event_t *)va;
    const perf_event_t *b = (const perf_event_t *)vb;
    return (int)(a->hash - b->hash);
}

static perf_event_t *perf_lookup(char *string, perf_event_t *user_table, int size)
{
    perf_event_t zero = {
        "None",
        0,
        0
    };
    uint32_t key = perf_hash(string);
    perf_event_t table = {"None", 0, key};
    perf_event_t *r = (perf_event_t *)bsearch(&table, user_table, size, sizeof(perf_event_t), &perf_cmp);
    return r ? r : NULL;
}

void init_events(void);

int get_events(void);

void begin_profiling(void);

uint64_t end_profiling(void);

int perf_list(const char *buf);

void perf_print(void);

int perf_cleanup(void);

#endif // _PERF_H_