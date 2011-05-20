#ifndef _PANIC_H
#define _PANIC_H

void panic(void) __attribute__((noreturn));
void gbc_panic(void) __attribute__((noreturn));

#endif
