#ifndef UNISTD_H
#define UNISTD_H

#include <sys/types.h>

pid_t fork(void); // GCC required
int execv(const char* path, char* const argv[]); // GCC required
int execve(const char* path, char* const argv[], char* const envp[]); // GCC required
int execvp(const char* file, char* const argv[]); // GCC required
pid_t getpid();

#endif
