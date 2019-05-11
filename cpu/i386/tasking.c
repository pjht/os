#include "tasking_helpers.h"
#include "tasking.h"
#include "../tasking.h"
#include "isr.h"
#include <stdio.h>
#include <string.h>
#include "kmalloc.h"
#include "memory.h"
#include "gdt.h"
#include "paging.h"
#include <stdint.h>
#include <stdlib.h>
#define STACK_PAGES 2

uint32_t next_pid;

static Task* currentTask;
static Task* headTask;
static Task* createTaskCr3(void* eip,void* cr3);

void tasking_init() {
  currentTask=NULL;
  next_pid=0;
  headTask=tasking_createTask(NULL);
  currentTask=headTask;
}

Task* tasking_createTaskCr3(void* eip,void* cr3) {
    Task* task=kmalloc(sizeof(Task));
    task->cr3=(uint32_t)cr3;
    uint32_t old_cr3;
    asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(old_cr3)::"%eax");
    load_address_space(task->cr3);
    task->esp=((uint32_t)alloc_pages(1))+0xfff;
    registers_t registers;
    registers.ds=0x23;
    registers.eip=(uint32_t)eip;
    registers.cs=0x1B;
    uint32_t eflags;
    asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(eflags)::"%eax");
    registers.eflags=eflags|0x200;
    registers.useresp=task->esp;
    registers.ss=0x23;
    char* interrupt_data_char=(char*)task->esp;
    char* registers_char=(char*)&registers;
    for (int i=0;i<sizeof(registers_t);i++) {
      interrupt_data_char[sizeof(registers_t)-i]=registers_char[i];
    }
    registers_t* interrupt_data=(registers_t*)task->esp;
    interrupt_data->ds=0x23;
    interrupt_data->eip=(uint32_t)eip;
    interrupt_data->cs=0x1B;
    asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(eflags)::"%eax");
    interrupt_data->eflags=eflags|0x200;
    interrupt_data->useresp=task->esp;
    interrupt_data->ss=0x23;
    load_address_space(old_cr3);
    task->next=NULL;
    task->pid=next_pid;
    task->priv=0;
    task->errno=0;
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

int* tasking_get_errno_address() {
  return &(currentTask->errno);
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
  return tasking_createTaskCr3(eip,paging_new_address_space());
}

void tasking_send_msg(uint32_t pid,void* msg,uint32_t size) {
  for (Task* task=headTask;task!=NULL;task=task->next) {
    if (task->pid==pid) {
      uint32_t cr3;
      asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(cr3)::"%eax");
      void* phys_addr=virt_to_phys(msg);
      load_address_space(task->cr3);
      uint32_t page=find_free_pages((size/4096)+1);
      map_pages((void*)(page<<12),phys_addr,(size/4096)+1,1,0);
      if (task->msg_store==NULL) {
        task->msg_store=malloc(sizeof(void*)*256);
        task->sender_store=malloc(sizeof(uint32_t)*256);
      }
      task->msg_store[task->wr]=(void*)(page<<12);
      task->sender_store[task->wr]=currentTask->pid;
      load_address_space(cr3);
      task->wr++;
      if (task->wr==task->rd) {
        task->wr--;
      }
      break;
    }
  }
}

void* tasking_get_msg(uint32_t* sender) {
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

void tasking_yield(registers_t registers) {
    Task* task=currentTask->next;
    if (!task) {
      task=headTask;
    }
    Task* oldCurr=currentTask;
    currentTask=task;
    oldCurr->esp=registers.useresp;
    registers_t* interrupt_data=(registers_t*)registers.useresp;
    load_address_space(task->cr3);
    char* interrupt_data_char=(char*)interrupt_data;
    char* registers_char=(char*)&registers;
    for (int i=0;i<sizeof(registers_t);i++) {
      interrupt_data_char[sizeof(registers_t)-i]=registers_char[i];
    }
    // memcpy(interrupt_data,&registers,sizeof(registers_t));
    if (task->priv) {
      allow_all_ports();
    } else {
      block_all_ports();
    }
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
      pop %eax; \
      or $0x200,%eax; \
      push %eax; \
      pushl $0x1B; \
      push $1f; \
      iret; \
    1: \
      ");
    switchTask(task->esp);
}
