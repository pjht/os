#include <stddef.h>
#include <memory.h>
#include <pthread.h>

pthread_spinlock_t lock;

int liballoc_lock() {
  return pthread_spin_lock(&lock);
}

extern int liballoc_unlock() {
  return pthread_spin_unlock(&lock);
}

extern void* liballoc_alloc(size_t num_pages) {
  return alloc_memory(num_pages);
}

extern int liballoc_free(void* ptr,size_t num_pages) {
  return 0;
}
