#include "tasking_helpers.h"
#include "tasking.h"
#include "../tasking.h"
#include "isr.h"
#include "../../libc/stdlib.h"
#include "../../libc/stdio.h"
#include "memory.h"
#include <stdint.h>
#define STACK_PAGES 2

uint32_t next_pid;

static Task* currentTask;
static Task* headTask;

void tasking_init() {
  currentTask=NULL;
  next_pid=0;
  headTask=createTask(NULL);
  currentTask=headTask;
}

Task* createTaskEax(void* eip,uint32_t eax) {
    Task* task=malloc(sizeof(Task));
    task->regs.eax=eax;
    task->regs.ebx=0;
    task->regs.ecx=0;
    task->regs.edx=0;
    task->regs.esi=0;
    task->regs.edi=0;
    asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(task->regs.eflags)::"%eax");
    task->regs.eip=(uint32_t)eip;
    asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(task->regs.cr3)::"%eax");
    task->regs.esp=(uint32_t)alloc_memory(1);
    task->regs.ebp=0;
    task->msg_store=NULL;
    task->rd=0;
    task->wr=0;
    task->next=NULL;
    task->pid=next_pid;
    next_pid++;
    if (currentTask) {
      currentTask->next=task;
    }
    return task;
}

Task* createTask(void* eip) {
  return createTaskEax(eip,0);
}

void send_msg(uint32_t pid,char* msg) {
  for (Task* task=headTask;task!=NULL;task=task->next) {
    if (task->pid==pid) {
      if (task->msg_store==NULL) {
        task->msg_store=malloc(sizeof(char*)*256);
        task->sender_store=malloc(sizeof(uint32_t)*256);
      }
      task->msg_store[task->wr]=msg;
      task->sender_store[task->wr]=currentTask->pid;
      task->wr++;
      if (task->wr==task->rd) {
        task->wr--;
      }
    }
  }
}

char* get_msg(uint32_t* sender) {
  if (!currentTask->msg_store) {
    return NULL;
  }
  if (currentTask->msg_store[currentTask->rd]==NULL) {
    currentTask->rd++;
    if (currentTask->msg_store[currentTask->rd]==NULL) {
      currentTask->rd--;
      return NULL;
    }
  }
  *sender=currentTask->sender_store[currentTask->rd];
  char* data=currentTask->msg_store[currentTask->rd];
  currentTask->msg_store[currentTask->rd]=NULL;
  currentTask->sender_store[currentTask->rd]=0;
  currentTask->rd++;
  if (currentTask->rd>currentTask->wr) {
    currentTask->rd=currentTask->wr-1;
  }
  return data;
}

uint32_t fork() {
  uint32_t eip=readEip();
  if (eip==0x12345) {
    printf("CHILD\n");
    return currentTask->pid;
  }
  Task* task=createTaskEax(eip,0x12345);
  return 0;
}

void yield() {
    Task* task=currentTask->next;
    if (!task) {
      task=headTask;
    }
    Task* oldCurr=currentTask;
    currentTask=task;
    switchTask(&oldCurr->regs, &currentTask->regs);
}
