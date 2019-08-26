#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>
#include <stdint.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void* malloc(size_t size);
void* realloc(void *mem, size_t new_sz);
void free(void* mem);
__attribute__((noreturn)) void exit(int code);

#endif
