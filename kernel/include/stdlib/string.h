#ifndef _STRING_H_
#define _STRING_H_

#include "types.h"
#include "err.h"

void *memcpy(void *dest, const void *src, size_t n);

void *memmove(void *dest, const void *src, size_t n);

int strlen(const char *s1);

int strncmp(const char *s1, const char *s2, uint32_t n);

int strcmp(const char *s1, const char *s2);

char *strncpy(char *dest, const char *src, uint32_t n);

char *strncat(char *dest, const char *src, uint32_t n);

int memcmp(const char *s1, const char *s2, uint32_t n);

int32_t strlcpy(char *dest, const char *src, uint32_t n);

#endif // _STRING_H_