#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "../drivers/isr.h"
#include <stdint.h>

void syscall_write_string(char* str);
void syscall_screen_backspace();
void syscall_register_interrupt_handler(uint8_t n,isr_t handler);
unsigned char syscall_port_byte_in(unsigned short port);

#endif
