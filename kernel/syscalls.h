#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "../drivers/isr.h"
#include <stdint.h>

void syscall_write_string(char* str);
void syscall_screen_backspace();
void syscall_gets(char* buf);

#endif
