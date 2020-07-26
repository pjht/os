#ifndef PTHREAD_H
#define PTHREAD_H

#include <sys/types.h>

typedef pid_t pthread_t;
typedef int pthread_attr_t; //Created as dummy

int pthread_create(pthread_t *restrict thread,
       const pthread_attr_t *restrict attr,
       void *(*start_routine)(void*), void *restrict arg);
#endif
