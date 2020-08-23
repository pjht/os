#ifndef SPAWN_H
#define SPAWN_H

#include <sys/types.h>

#define posix_spawn_file_actions_t int
#define posix_spawnattr_t int

int posix_spawn(pid_t* pid, const char* path, const posix_spawn_file_actions_t* file_actions, const posix_spawnattr_t* attrp,
char* const argv[], char* const envp[]);

#endif
