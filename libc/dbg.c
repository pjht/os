void serial_print(char* str) {
  asm volatile("  \
    mov $16, %%eax; \
    int $80; \
  "::"b"(str));
}
