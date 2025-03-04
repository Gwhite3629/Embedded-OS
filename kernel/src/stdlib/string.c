#include <stdlib.h>
#include <memory/malloc.h>
#include <trace/strace.h>

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
    char *d = dest;
	const char *s = src;

	if (d==s) return d;
	if ((uintptr_t)s-(uintptr_t)d-n <= -2*n) return memcpy(d, s, n);

	if (d<s) {
		for (; n; n--) *d++ = *s++;
	} else {
		while (n) n--, d[n] = s[n];
	}

    return dest;
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

char *strcpy(char *dest, const char *src)
{
    int i = 0;
    char c = '\0';
    do {
        c = src[i];
        dest[i] = c;
        i++;
    } while (c != '\0');

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
    push_trace("char *strdup(const char *)","strdup",src,0,0,0);
    int len = strlen(src) + 1;
    char *dst = NULL;
    new(dst, len, char);
    memcpy(dst, src, len-1);
exit:
    pop_trace();
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

char **str_split(const char *str, char delim, unsigned int *n_token)
{
    char **sep_str = NULL;
    char *host_str = NULL;
    char *start = NULL;
    char *end = NULL;
    
    char c;
    int start_off = 0;
    int end_off = strlen(str);

    // Clear leading and trailing delim characters
    while (str[start_off] == delim) {
        start_off++;
    }

    while (str[end_off - 1] == delim) {
        end_off--;
    }

    // +2 because of trailing null and end_off -1
    new(host_str, end_off - start_off + 2, char);
    strncpy(host_str, str + start_off, end_off - start_off + 1);
   
    printk("host_str %s\n", host_str);
    
    start = host_str;

    new(sep_str, 1, char *);
    *n_token = 0;

    int j = strlen(host_str);
    for (int i = 0; i < j; i++) {
        if (host_str[i] == delim) {
            while (host_str[i + 1] == delim) {
                j -= 1;
                memmove(&host_str[i],&host_str[i+1],j-i);
                host_str[j] = '\0';
            }
        }
    }

    while (c = *host_str++) {
        if (c == delim) {
            (*n_token)++;
            alt(sep_str, *n_token + 1, char *);
            sep_str[*n_token - 1] = start;
            start = host_str;
            host_str[-1] = '\0';
        }
    }

    (*n_token)++;
    alt(sep_str, (*n_token + 1), char *);
    sep_str[*n_token - 1] = start;

exit:
    return sep_str;
}

char *list2str(list_t *list, char *delim)
{
    int ret = E_NOERR;
    char *val = NULL;
    new(val, 256, char);
    memset(val, 0, 256);
    int len = 0, val_len = 256;
    while(list_size(list)> 0) {
        char * temp = list_pop(list)->value;
        int len_temp = strlen(temp);
        if(len + len_temp + 1 + 1 > val_len) {
            val_len = val_len * 2;
            alt(val, val_len, char);
            len = len + len_temp + 1;
        }
        strncat(val, delim, strlen(delim));
        strncat(val, temp, strlen(temp));
    }
exit:
    return (ret == E_NOERR) ? val : NULL;
}
