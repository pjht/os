void yield() {
  asm volatile("  \
    mov $1, %eax; \
    int $80; \
  ");
}
