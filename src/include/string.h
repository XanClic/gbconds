#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>
#include <stdint.h>

void *memcpy(void *d, const void *s, size_t size);
void *memset(void *d, int c, size_t size);
void *memsetptr(void *s, uintptr_t c, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);

#endif

