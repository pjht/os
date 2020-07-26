#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void* malloc(size_t size);
void* realloc(void *mem, size_t new_sz);
void free(void* mem);
void abort(void);               // GCC required
int atexit(void (*func)(void)); // GCC required
int atoi(const char *str);      // GCC required
char *getenv(const char *name); // GCC required
__attribute__((noreturn)) void exit(int code);

#endif
