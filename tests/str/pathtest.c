#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "strsep.h"

static inline uint32_t hash32(char *str)
{
    unsigned long val = 5381;
    int c = 0;
    while (c = *str++) {
        val = ((val << 5) + val) + c;
    }
    return (uint32_t)(val & (__INT_MAX__));
}

int main(int argc, char *argv[])
{
    char **sep_str = NULL;
    unsigned int n_token = 0;

    if (argc < 2) {
        printf("Incorrect usage:\n\t%s <path>\n", argv[0]);
        return -1;
    }

    sep_str = str_split(argv[1], '/', &n_token);

    printf("n_token: %d\n", n_token);
    printf("test hashes: %8x, %8x\n", hash32("tdirq"), hash32("tdir "));

    //printf("\n");
    for (int i = 0; i < n_token; i++) {
        printf("\t%s %8x\n", sep_str[i], hash32(sep_str[i]));
    }

exit:
    if (sep_str[0]) {
        free(sep_str[0]);
    }
    if (sep_str) {
        free(sep_str);
    }
    return 0;
}
