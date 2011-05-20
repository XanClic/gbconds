#include <stddef.h>
#include <stdint.h>
#include <string.h>

void *memset(void *s, int c, size_t n) __attribute__((weak));
void *memset(void *s, int c, size_t n)
{
    uint8_t *d = s;

    while (n--)
        *(d++) = c;

    return s;
}

void *memsetptr(void *s, uintptr_t c, size_t n) __attribute__((weak));
void *memsetptr(void *s, uintptr_t c, size_t n)
{
    uintptr_t *d = s;

    while (n--)
        *(d++) = c;

    return s;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    while (n--)
    {
        if (*(s1++) != *s2)
            return (unsigned char)s2[0] - (unsigned char)s1[-1];
        if (!*(s2++))
            return 0;
    }

    return 0;
}

int strcmp(const char *s1, const char *s2)
{
    int ret;

    while (!(ret = (unsigned char)*(s2++) - (unsigned char)*s1))
        if (!*(s1++))
            return 0;

    return ret;
}

int memcmp(const void *m1, const void *m2, size_t n)
{
    int ret = 0;
    const char *s1 = m1, *s2 = m2;

    while (n-- && !(ret = (unsigned char)*(s2++) - (unsigned char)*(s1++)));

    return ret;
}

void *memcpy(void *restrict d, const void *restrict s, size_t n) __attribute__((weak));
void *memcpy(void *restrict d, const void *restrict s, size_t n)
{
    const uint8_t *s8 = s;
    uint8_t *d8 = d;

    while (n--)
        *(d8++) = *(s8++);

    return d;
}

char *strcpy(char *restrict d, const char *restrict s)
{
    char *vd = d;

    while (*s)
        *(vd++) = *(s++);
    *vd = 0;

    return d;
}

char *strncpy(char *restrict d, const char *restrict s, size_t n)
{
    char *vd = d;

    if (!n)
        return d;

    n++;
    while (--n && *s)
        *(vd++) = *(s++);
    while (n--)
        *(vd++) = 0;

    return d;
}

char *strncpy0(char *restrict d, const char *restrict s, size_t n)
{
    char *vd = d;

    n++;
    while ((--n > 1) && *s)
        *(vd++) = *(s++);
    while (n--)
        *(vd++) = 0;

    return d;
}

char *strcat(char *restrict d, const char *restrict s)
{
    char *vd = d;

    while (*(vd++));
    vd--;

    while (*s)
        *(vd++) = *(s++);
    *vd = 0;

    return d;
}

size_t strlen(const char *s)
{
    size_t i;
    for (i = 0; s[i]; i++);
    return i;
}

size_t strnlen(const char *s, size_t mlen)
{
    size_t i;
    for (i = 0; s[i] && (i < mlen); i++);
    return i;
}

char *strchr(const char *s, int c)
{
    while ((*s != c) && *s)
        s++;

    if (!*s)
        return NULL;
    return (char *)s;
}

char *strstr(const char *s1, const char *s2)
{
    int i;

    while (*s1)
    {
        for (i = 0; s2[i] && (s1[i] == s2[i]); i++);

        if (!s2[i])
            return (char *)s1;

        s1++;
    }

    return NULL;
}

char *strtok(char *string1, const char *string2)
{
    static char *next_token = NULL;
    char *tok;
    int slen = strlen(string2);

    if (string1 == NULL)
        string1 = next_token;

    if (string1 == NULL)
        return NULL;

    while ((tok = strstr(string1, string2)) == string1)
        string1 += slen;

    if (!*string1)
    {
        next_token = NULL;
        return NULL;
    }

    if (tok == NULL)
        next_token = NULL;
    else
    {
        *tok = '\0';
        next_token = tok + slen;
    }

    return string1;
}

int strlencmp(const char *s1, const char *s2, size_t s1_len)
{
    int ret = strncmp(s1, s2, s1_len);
    if (ret)
        return ret;
    return -s2[s1_len];
}
