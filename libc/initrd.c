long initrd_sz() {
  long size;
  asm volatile("  \
    mov $18, %%eax; \
    int $80; \
  ":"=b"(size));
  return size;
}

void initrd_get(char* initrd) {
  asm volatile("  \
    mov $19, %%eax; \
    int $80; \
  "::"b"(initrd));
}
