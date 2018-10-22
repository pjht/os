#include "../drivers/isr.h"
#include <stdint.h>

void syscall_write_string(char* str) {
  asm volatile("int $80"::"a"(0),"b"(str));
}

void syscall_screen_backspace() {
  asm volatile("int $80"::"a"(1));
}

void syscall_gets(char* buf) {
  asm volatile("int $80"::"a"(2),"b"(buf));
}
