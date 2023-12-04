#ifndef _STRING_H_
#define _STRING_H_

#include "types.h"
#include "err.h"

extern void *memcpy(void *dest, const void *src, size_t n);

extern void *memmove(void *dest, const void *src, size_t n);

extern int strlen(const char *s1);

extern int strncmp(const char *s1, const char *s2, uint32_t n);

extern char *strncpy(char *dest, const char *src, uint32_t n);

extern char *strncat(char *dest, const char *src, uint32_t n);

extern int memcmp(const char *s1, const char *s2, uint32_t n);

extern int32_t strlcpy(char *dest, const char *src, uint32_t n);

#endif // _STRING_H_