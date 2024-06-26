#include <stdlib.h>

void *memcpy(void *dest, const void *src, size_t n)
{
    char *bsrc = (char *)src;
    char *bdest = (char *)dest;

    if (bdest == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < n; i++) {
        bdest[i] = bsrc[i];
    }

    return bdest;
}

void *memmove(void *dest, const void *src, size_t n)
{
    char *bsrc = (char *)src;
    char *bdest = (char *)dest;

    if (bdest == NULL) {
        return NULL;
    }

    char temp[n];

    for (size_t i = 0; i < n; i++) {
        temp[i] = bsrc[i];
    }

    for (size_t i = 0; i < n; i++) {
        bdest[i] = temp[i];
    }

    return bdest;
}

int strlen(const char *s1)
{
    int i = 0;

    while (s1[i] != 0) i++;

    return i;
}

int strncmp(const char *s1, const char *s2, uint32_t n)
{
    int i = 0;
    int r;

    while (n > 0) {
        r = s1[i] - s2[i];
        if (r != 0) return r;
        else if (s1[i]==0) return 0;

        i++;
        n--;
    }

    return 0;
}

int strcmp(const char *s1, const char *s2)
{
    while(*s1 == *s2++) {
        if (*s1++ == '\0') {
            return 0;
        }
    }
    return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}

char *strncpy(char *dest, const char *src, uint32_t n)
{
    uint32_t i;

    for (i = 0; i < n; i++) {
        dest[i] = src[i];
        if (src[i]=='\0') break;
    }
    for (;i < n; i++) {
        dest[i] = '\0';
    }

    return dest;
}

char *strncat(char *dest, const char *src, uint32_t n)
{
    uint32_t i;
    uint32_t dest_len;

    dest_len = strlen(dest);

    for (i = 0; i < n; i++) {
        dest[dest_len + i] = src[i];
        if (src[i]=='\0') break;
    }
    dest[dest_len+i] = '\0';

    return dest;
}

int memcmp(const char *s1, const char *s2, uint32_t n)
{
    int i = 0;
    int r;

    while (1) {
        if (i==n) return 0;

        r = s1[i] - s2[i];
        if (r != 0) return r;

        i++;
    }

    return 0;
}

int32_t strlcpy(char *dest, const char *src, uint32_t n)
{
    uint32_t i;

    for (i = 0; i < (n - 1); i++) {
        dest[i] = src[i];
        if (src[i] == '\0') break;
    }
    dest[i] = '\0';

    return i;
}