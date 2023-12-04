#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "../../include/qsort.h"
#include "../../include/bsearch.h"

typedef struct command_t {
    char name[32];
    int hash;
	int id;
} command_t;

#define TABLESIZE 12

extern command_t COMMANDS;

static inline unsigned long
hash(char *name)
{
	unsigned long val = 5381;
	int c;
	while ((c = *name++)) {
		val = ((val << 5) + val) + c;
	}
	return (int)(val % (__INT_MAX__));
}

static inline int
cmp(const void *va, const void *vb)
{
	const command_t *a = (command_t *)va;
	const command_t *b = (command_t *)vb;
	return (int)(a->hash - b->hash);
}

static inline void
init_table(void) {
	qsort(&COMMANDS, TABLESIZE, sizeof(command_t), (__compar_d_fn_t)&cmp);
}

static inline command_t
lookup(char *name)
{
	command_t zero = {
		"None",
		0,
		-1
	};
	int key = hash(name);
	command_t table = {"None",key,-1};
	command_t *r = (command_t *)bsearch(&table, &COMMANDS, TABLESIZE, sizeof(command_t), &cmp);
	return r ? *r : zero;
}

static inline command_t
table_lookup(char *name, command_t *user_table, int size)
{
	command_t zero = {
		"None",
		0,
		-1
	};
	int key = hash(name);
	command_t table = {"None",key,-1};
	command_t *r = (command_t *)bsearch(&table, user_table, size, sizeof(command_t), &cmp);
	return r ? *r : zero;
}

#endif // COMMANDS_H
