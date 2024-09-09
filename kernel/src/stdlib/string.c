#include <stdlib.h>
#include <memory/malloc.h>

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

char *strdup(const char *src)
{
    int len = strlen(src) + 1;
    char *dst = NULL;
    new(dst, len, char);
    memcpy(dst, src, len);
exit:
    return dst;
}

char *strstr(const char *in, const char *str)
{
    char c;
    uint32_t len;

    c = *str++;
    if (!c)
        return (char *)in;
    len = strlen(str);
    do {
        char sc;

        do {
            sc = *in++;
            if (!sc)
                return (char *) 0;
        } while (sc != c);
    } while (strncmp(in, str, len) != 0);

    return (char *) (in - 1);
}

void itoa(char *buf, unsigned long int n, int base)
{
    unsigned long int tmp;
    int i, j;

    tmp = n;
    i = 0;

    do {
        tmp = n % base;
        buf[i++] = (tmp < 10) ? (tmp + '0') : (tmp + 'a' - 10);
    } while (n /= base);
    buf[i--] = 0;

    for (j = 0; j < i; j++, i--) {
        tmp = buf[j];
        buf[j] = buf[i];
        buf[i] = tmp;
    }
}

int atoi(char *string)
{
    int result = 0;
    unsigned int digit;
    int sign;

    while (isspace(*string)) {
        string += 1;
    }

    /*
     * Check for a sign.
     */

    if (*string == '-') {
        sign = 1;
        string += 1;
    } else {
        sign = 0;
        if (*string == '+') {
            string += 1;
        }
    }

    for ( ; ; string += 1) {
        digit = *string - '0';
        if (digit > 9) {
            break;
        }
        result = (10*result) + digit;
    }

    if (sign) {
        return -result;
    }
    return result;
}

int isspace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}
int isprint(char c)
{
    return ((c >= ' ' && c <= '~') ? 1 : 0);
}

char *strsep(char **stringp, const char *delim)
{
    char *s;
    const char *spanp;
    int c, sc;
    char *tok;
    if ((s = *stringp) == NULL)
        return (NULL);
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
}

list_t *str_split(const char *str, const char *delim, unsigned int *n_token)
{
    list_t *ret_list = list_create();
    char *s = strdup(str);
    char *token, *rest = s;
    while ((token = strsep(&rest, delim)) != NULL) {
        if(!strcmp(token, ".")) continue;
        if(!strcmp(token, "..")) {
            if(list_size(ret_list) > 0) list_pop(ret_list);
            continue;
        }
        list_push(ret_list, strdup(token));
        if(n_token) (*n_token)++;
    }
    del(s);
exit:
    return ret_list;
}

char *list2str(list_t *list, char *delim)
{
    char *ret = NULL;
    new(ret, 256, char);
    memset(ret, 0, 256);
    int len = 0, ret_len = 256;
    while(list_size(list)> 0) {
        char * temp = list_pop(list)->value;
        int len_temp = strlen(temp);
        if(len + len_temp + 1 + 1 > ret_len) {
            ret_len = ret_len * 2;
            alt(ret, ret_len, char);
            len = len + len_temp + 1;
        }
        strncat(ret, delim, strlen(delim));
        strncat(ret, temp, strlen(temp));
    }
exit:
    return ret;
}