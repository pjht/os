#ifndef INT_TASKING_H
#define INT_TASKING_H

#include <stdint.h>


typedef struct Task {
  uint32_t kernel_esp;
  uint32_t kernel_esp_top;
  uint32_t cr3;
  uint32_t user_esp;
  char priv;
  int errno;
  uint32_t pid;
  char** msg_store;
  uint32_t* sender_store;
  uint32_t msg_indx;
  uint8_t rd;
  uint8_t wr;
  struct Task* next;
} Task;

int* tasking_get_errno_address();

#endif
