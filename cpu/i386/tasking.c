#include "paging.h"
#include "paging_helpers.h"
#include "tasking_helpers.h"
#include "isr.h"
#include "../../libc/stdlib.h"
#include <stdint.h>
#define STACK_PAGES 2

typedef struct Task {
    uint32_t esp,ebp;
    uint32_t eip,cr3;
    struct Task *next;
    uint32_t pid;
} Task;

Task* tasks_tail=NULL;
Task* tasks_head=NULL;
Task* current_task=NULL;
uint32_t next_pid=0;

void tasking_init() {
  Task* task=malloc(sizeof(Task*));
  task->cr3=((uint32_t)page_directory)-0xC0000000;
  task->next=NULL;
  task->pid=next_pid;
  next_pid++;
  tasks_tail=task;
  tasks_head=task;
  current_task=task;
}


uint32_t fork() {
  Task* task=malloc(sizeof(Task*));
  task->cr3=current_task->cr3;
  task->next=NULL;
  task->pid=next_pid;
  next_pid++;
  uint32_t eip=read_eip();
  if (task==current_task) {
    return 0;
  } else {
    task->eip=eip;
    asm volatile("mov %%esp, %0" : "=r"(task->esp));
    asm volatile("mov %%ebp, %0" : "=r"(task->ebp));
    tasks_tail->next=task;
    return task->pid;
  }
}

void yield() {
  if (!current_task) {
    return;
  }
  Task* next=current_task->next;
  if (next==0) {
    next=tasks_head;
  }
  uint32_t eip=read_eip();
  if (eip==0x12345) {
    return;
  }
  current_task->eip=eip;
  asm volatile("mov %%esp, %0" : "=r"(current_task->esp));
  asm volatile("mov %%ebp, %0" : "=r"(current_task->ebp));
  current_task=next;
  load_page_directory((uint32_t*)next->cr3);
  set_regs(next->esp,next->ebp,next->eip);
}
