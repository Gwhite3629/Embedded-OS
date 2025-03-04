#ifndef _STRING_H_
#define _STRING_H_

#include "list.h"
#include "types.h"
#include "err.h"

void *memcpy(void *dest, const void *src, size_t n);

void *memmove(void *dest, const void *src, size_t n);

int strlen(const char *s1);

int strncmp(const char *s1, const char *s2, uint32_t n);

int strcmp(const char *s1, const char *s2);

char *strncpy(char *dest, const char *src, uint32_t n);

char *strcpy(char *dest, const char *src);

char *strncat(char *dest, const char *src, uint32_t n);

int memcmp(const char *s1, const char *s2, uint32_t n);

int32_t strlcpy(char *dest, const char *src, uint32_t n);

char *strdup(const char *src);

char *strstr(const char *in, const char *str);

void itoa(char *buf, unsigned long int n, int base);

int atoi(char *string);

int isspace(char c);
int isprint(char c);

char *strsep(char **stringp, const char *delim);
char **str_split(const char *str, char delim, unsigned int *n_token);
char *list2str(list_t *list, char *delim);

#endif // _STRING_H_
