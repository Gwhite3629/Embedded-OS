#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "read.h"
#include "help.h"

int readl(char *line, int *n_tokens, char ***tokens) {
    int r = 0;
    char **tmp = NULL;

    memset(line, 0, MAX_LINE);

    fgets(line, MAX_LINE, stdin);

    r = parse(line, n_tokens, tokens);

    tmp = realloc((*tokens), sizeof(char *) * ((*n_tokens)+1));
    if (tmp == NULL) {
        printf("Failed to reallocate\n");
        printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
        perror(strerror(errno));
        return -1;
    }
    (*tokens) = tmp;
    (*tokens)[(*n_tokens)] = NULL;

    return r;
}

int parse(char *line, int *n_tokens, char ***tokens)
{
    int r = 0;

    char *t = line;
    int wc = 1;
    int c_count = 0;
    char **tmp = NULL;

    while(t != NULL) {
        if (*t == ' ') {
            wc++;
            tmp = realloc((*tokens), sizeof(char *) * (wc));
            if (tmp == NULL) {
                printf("Failed to reallocate\n");
                printf("%s: %s: %d\n", __FILE__, __func__, __LINE__);
                perror(strerror(errno));
                return -1;
            }
            (*tokens) = tmp;
            (*tokens)[wc-1] = malloc(sizeof(char)*32);
            memset((*tokens)[wc-1], 0, sizeof(char)*32);
            c_count = 0;
        } else if (*t == '\n') {
            (*n_tokens) = wc;
            return r;
        }
        else {
            (*tokens)[wc-1][c_count] = *t;
            c_count++;
            (*tokens)[wc-1][c_count] = '\0';
        }
        t++;
    }
    (*n_tokens) = wc;
    return r;
}