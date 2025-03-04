#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "strsep.h"

char **str_split(const char *str, char delim, unsigned int *n_token)
{
    char **sep_str = NULL;
    char *host_str = NULL;
    char *start = NULL;
    char *end = NULL;
    
    char c;
    int start_off = 0;
    int end_off = strlen(str) - 1;

    // Clear leading and trailing delim characters
    while (str[start_off] == delim) {
        start_off++;
    }

    printf("start_off: %d\n", start_off);

    while (str[end_off] == delim) {
        end_off--;
    }

    printf("end_off: %d\n", end_off);

    // +2 because of trailing null and end_off -1
    host_str = (char *)malloc((end_off - start_off + 1) * sizeof(char));
    strncpy(host_str, str + start_off, end_off - start_off + 1);
    
    start = host_str;

    sep_str = (char **)malloc(sizeof(char *));
    *n_token = 0;

    int j = strlen(host_str);
    for (int i = 0; i < j; i++) {
        if (host_str[i] == delim) {
            while (host_str[i + 1] == delim) {
                j -= 1;
                memmove(&host_str[i],&host_str[i+1], j - i);
                host_str[j] = '\0';
            }
        }
    }

    while (c = *host_str++) {
        if (c == delim) {
            (*n_token)++;
            sep_str = realloc(sep_str, (*n_token + 1) * sizeof(char *));
            sep_str[*n_token - 1] = start;
            start = host_str;
            host_str[-1] = '\0';
        }
    }

    (*n_token)++;
    sep_str = realloc(sep_str, (*n_token + 1) * sizeof(char *));
    sep_str[*n_token - 1] = start;

exit:
    return sep_str;
}

