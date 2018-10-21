#include "../drivers/isr.h"
#include <stdint.h>

void syscall_write_string(char* str) {
  asm volatile("int $80"::"a"(0),"b"(str));
}

void syscall_screen_backspace() {
  asm volatile("int $80"::"a"(1));
}

void syscall_register_interrupt_handler(uint8_t n,isr_t handler) {
  asm volatile("int $80"::"a"(2),"b"(n),"c"(isr_handler));
}

unsigned char syscall_port_byte_in(unsigned short port) {
  unsigned char result;
  asm volatile("int $80":"=a"(result):"a"(3),"b"(port));
  return result;
}
