#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void* malloc(size_t size);
void free(void* mem);

#endif
