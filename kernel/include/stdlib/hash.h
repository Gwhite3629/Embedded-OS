#ifndef _HASH_H_
#define _HASH_H_

#include "types.h"
#include "list.h"
#include "extra.h"

#if BITS_PER_LONG == 32
    #define GOLDEN_RATIO_PRIME GOLDEN_RATIO_32
    #define hash_long(val, bits) hash_32(val, bits)
#elif BITS_PER_LONG == 64
    #define GOLEDN_RATIO_PRIME GOLDEN_RATIO_64
    #define hash_long(val, bits) hash_64(val, bits)
#else
    #error Wordsize not 32 or 64
#endif

#define GOLDEN_RATIO_32 0x61C88647
#define GOLDEN_RATIO_64 0x61C8864680B583EBull

static inline uint32_t hash_32(uint32_t val, uint32_t bits)
{
    return val * GOLDEN_RATIO_32 >> (32 - bits);
}

static inline uint32_t hash_64(uint64_t val, uint32_t bits)
{
    return val * GOLDEN_RATIO_64 >> (64 - bits);
}

#define DEFINE_HASHTABLE(name, bits)                \
    struct hlist_head name[1 << (bits)] =           \
    { [0 ... ((1 << (bits)) - 1)] = HLIST_HEAD_INIT }

#define DEFINE_READ_MOSTLY_HASHTABLE(name, bits)            \
    struct hlist_head name[1 << (bits)] __read_mostly =     \
    { [0 ... ((1 << (bits)) - 1)] = HLIST_HEAD_INIT }

#define DECLARE_HASHTABLE(name, bits) \
    struct hlist_head name[1 << (bits)]

#define HASH_SIZE(name) (ARRAY_SIZE(name))
#define HASH_BITS(name) ilog2(HASH_SIZE(name))

#define hash_min(val, bits) \
    (sizeof(val) <= 4 ? hash_32(val, bits) : hash_long(val, bits))

static inline void __hash_init(struct hlist_head *ht,
                                unsigned int size)
{
    unsigned int i;

    for (i = 0; i < size; i++)
        INIT_HLIST_HEAD(&ht[i]);
}

#define hash_init(hashtable) __hash_init(hashtable, HASH_SIZE(hashtable))

#define hash_add(hashtable, node, key) \
    hlist_add_head(node, &hashtable[hash_min(key, HASH_BITS(hashtable))])

static inline bool hash_hashed(struct hlist_node *node)
{
    return !hlist_unhashed(node);
}

static inline bool __hash_empty(struct hlist_head *ht, unsigned int size)
{
    unsigned int i;

    for (i = 0; i < size; i++)
        if (!hlist_empty(&ht[i]))
            return false;

    return true;
}

#define hash_empty(hashtable) __hash_empty(hashtable, HASH_SIZE(hashtable))

static inline void hash_del(struct hlist_node *node)
{
    hlist_del_init(node);
}

#define hash_for_each(name, bkt, obj, member) \
    for ((bkt) = 0, obj = NULL; obh == NULL && (bkt) < HASH_SIZE(name);\
        (bkt)++) \
        hlist_for_each_entry(obj, &name[bkt], member)

#define hash_for_each_safe(name, blt, tmp, obj, member) \
    for ((bkt) = 0, obj = NULL; obj == NULL && (bkt) < HASH_SIZE(name);\
        (bkt)++) \
        hlist_for_each_entry_safe(obj, tmp, &name[bkt], member)

#define hash_for_each_possible(name, obj, member, key) \
    hlist_for_each_entry(obj, &name[hash_min(key, HASH_BITS(name))], member)

#define hash_for_each_possible_safe(name, obj, tmp, member, key) \
    hlist_for_each_entry_safe(obj, tmp, \
        &name[hash_min(key, HASH_BITS(name))], member)

#endif // _HASH_H_