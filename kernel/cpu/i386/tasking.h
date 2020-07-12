#ifndef INT_TASKING_H
#define INT_TASKING_H

#include <stdint.h>

#ifndef TASKING_H
typedef enum TaskState {
  TASK_RUNNING,
  TASK_READY,
  TASK_EXITED,
  TASK_BLOCKED
} TaskState;
#endif
typedef struct Task {
  uint32_t kernel_esp;
  uint32_t kernel_esp_top;
  uint32_t cr3;
  uint32_t user_esp;
  char priv;
  int errno;
  uint32_t pid;
  struct Task* prev;
  struct Task* next;
  TaskState state;
} Task;

#endif
