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

extern void task_init();


uint32_t next_pid;

Task* currentTask;
static Task* headTask;
Task* tasking_createTaskCr3Kmode(void* eip,void* cr3,char kmode);

void tasking_init(void* esp) {
  currentTask=NULL;
  next_pid=0;
  headTask=tasking_createTaskCr3Kmode(NULL,paging_new_address_space(),1);
  currentTask=headTask;
}

Task* tasking_createTaskCr3Kmode(void* eip,void* cr3,char kmode) {
    Task* task=kmalloc(sizeof(Task));
    task->cr3=(uint32_t)cr3;
    uint32_t old_cr3;
    asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(old_cr3)::"%eax");
    load_address_space(task->cr3);
    task->kernel_esp=(((uint32_t)alloc_pages(1))+0xfff);
    task->kernel_esp_top=task->kernel_esp;
    if (kmode) {
      task->kernel_esp-=5*4;
      uint32_t* stack_top_val=(uint32_t*)task->kernel_esp;
      stack_top_val[0]=0;
      stack_top_val[1]=0;
      stack_top_val[2]=0;
      stack_top_val[3]=0;
      stack_top_val[4]=eip;
    } else {
      task->kernel_esp-=7*4;
      uint32_t* stack_top_val=(uint32_t*)task->kernel_esp;
      stack_top_val[0]=0;
      stack_top_val[1]=0;
      stack_top_val[2]=0;
      stack_top_val[3]=0;
      stack_top_val[4]=task_init;
      stack_top_val[5]=(((uint32_t)alloc_pages(1))+0xfff);;
      stack_top_val[6]=eip;
    }
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
  return tasking_createTaskCr3Kmode(eip,paging_new_address_space(),0);
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
  switch_to_task(task);
}
