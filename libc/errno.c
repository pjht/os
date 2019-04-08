int* __get_errno_address() {
  int* address;
  asm volatile("  \
    mov $5, %%eax; \
    int $80; \
  ":"=b"(address):);
  return address;
}
