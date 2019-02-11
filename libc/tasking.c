void yield() {
  asm volatile("  \
    mov $1, %eax; \
    int $80; \
  ");
}

void createTask(void* task) {
  asm volatile("  \
    mov $2, %%eax; \
    int $80; \
  "::"b"(task));
}
