/*
pop %eax; \
or $0x200,%eax; \
push %eax; \
*/
#include "tasking_helpers.h"
#include "tasking.h"
#include "../tasking.h"
#include "isr.h"
#include <stdio.h>
#include "kmalloc.h"
#include "memory.h"
#include "gdt.h"
#include <stdint.h>
#define STACK_PAGES 2

uint32_t next_pid;

static Task* currentTask;
static Task* headTask;
static Task* createTaskKmode(void* eip,char kmode);

void tasking_init() {
  currentTask=NULL;
  next_pid=0;
  headTask=createTaskKmode(NULL,1);
  currentTask=headTask;
}

static Task* createTaskKmode(void* eip,char kmode) {
    Task* task=kmalloc(sizeof(Task));
    task->kmode=kmode;
    task->regs.eax=0;
    task->regs.ebx=0;
    task->regs.ecx=0;
    task->regs.edx=0;
    task->regs.esi=0;
    task->regs.edi=0;
    asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(task->regs.eflags)::"%eax");
    task->regs.eip=(uint32_t)eip;
    task->regs.cr3=new_address_space();
    uint32_t cr3;
    asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(cr3)::"%eax");
    load_address_space(task->regs.cr3);
    task->regs.esp=((uint32_t)alloc_memory(1))+0xfff;
    load_address_space(cr3);
    task->regs.ebp=0;
    task->msg_store=NULL;
    task->rd=0;
    task->wr=0;
    task->next=NULL;
    task->pid=next_pid;
    task->priv=0;
    if (currentTask) {
      task->priv=currentTask->priv;
    }
    if (task->pid==1) {
      task->priv=1;
    }
    next_pid++;
    if (currentTask) {
      currentTask->next=task;
    }
    return task;
}

char isPrivleged(uint32_t pid) {
  for (Task* task=headTask;task!=NULL;task=task->next) {
    if (task->pid==pid) {
      return task->priv;
    }
  }
  return 0;
}

Task* tasking_createTask(void* eip) {
  return createTaskKmode(eip,1);
}

void send_msg(uint32_t pid,void* msg) {
  for (Task* task=headTask;task!=NULL;task=task->next) {
    if (task->pid==pid) {
      if (task->msg_store==NULL) {
        task->msg_store=malloc(sizeof(void*)*256);
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

void* get_msg(uint32_t* sender) {
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

void tasking_yield() {
    Task* task=currentTask->next;
    if (!task) {
      task=headTask;
    }
    Task* oldCurr=currentTask;
    currentTask=task;
    load_address_space(task->regs.cr3);
    if (!task->kmode) {
      asm volatile("  \
        cli; \
        mov $0x23, %ax; \
        mov %ax, %ds; \
        mov %ax, %es; \
        mov %ax, %fs; \
        mov %ax, %gs; \
                      \
        mov %esp, %eax; \
        pushl $0x23; \
        pushl %eax; \
        pushf; \
        pushl $0x1B; \
        push $1f; \
        iret; \
      1: \
        ");
    }
    if (task->priv) {
      allow_all_ports();
    } else {
      block_all_ports();
    }
    switchTask(&oldCurr->regs, &currentTask->regs);
}
