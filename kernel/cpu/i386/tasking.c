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
static uint32_t* kstacks=(char*)0xF6800000;


uint32_t next_pid;

Task* currentTask;
static Task* headTask;
Task* tasking_createTaskCr3KmodeParam(void* eip,void* cr3,char kmode,char param1_exists,uint32_t param1_arg,char param2_exists,uint32_t param2_arg);

void tasking_init(void* esp) {
  currentTask=NULL;
  next_pid=0;
  headTask=tasking_createTaskCr3KmodeParam(NULL,paging_new_address_space(),1,0,0,0,0);
  currentTask=headTask;
}

Task* tasking_createTaskCr3KmodeParam(void* eip,void* cr3,char kmode,char param1_exists,uint32_t param1_arg,char param2_exists,uint32_t param2_arg) {
    Task* task=kmalloc(sizeof(Task));
    map_kstack(next_pid);
    uint32_t param1;
    if (param1_exists) {
      param1=param1_arg;
    } else {
      param1=1;
    }
    uint32_t param2;
    if (param2_exists) {
      param2=param2_arg;
    } else {
      param2=2;
    }
    task->cr3=(uint32_t)cr3;
    uint32_t old_cr3;
    asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(old_cr3)::"%eax");
    load_address_space(task->cr3);
    if (kmode) {
      uint32_t top_idx=(1024*(next_pid+1));
      task->kernel_esp=((uint32_t)(&kstacks[top_idx-5]));
      task->kernel_esp_top=task->kernel_esp;
      kstacks[top_idx-5]=0;
      kstacks[top_idx-4]=0;
      kstacks[top_idx-3]=0;
      kstacks[top_idx-2]=0;
      kstacks[top_idx-1]=eip;
    } else {
      uint32_t top_idx=(1024*(next_pid+1));
      task->kernel_esp=((uint32_t)(&kstacks[top_idx-7]));
      task->kernel_esp_top=task->kernel_esp;
      kstacks[top_idx-7]=0;
      kstacks[top_idx-6]=0;
      kstacks[top_idx-5]=0;
      kstacks[top_idx-4]=0;
      kstacks[top_idx-3]=task_init;
      uint32_t* user_stack=(((uint32_t)alloc_pages(1))+0x1000);
      user_stack-=2;
      user_stack[0]=param1;
      user_stack[1]=param2;
      kstacks[top_idx-2]=user_stack;
      kstacks[top_idx-1]=eip;
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
    if (next_pid>1024*32) {
      halt(); //Cannot ever create more than 32k tasks, as I don't currently reuse PIDs.
    }
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
  return tasking_createTaskCr3KmodeParam(eip,paging_new_address_space(),0,0,0,0,0);
}

void tasking_send_msg(uint32_t pid,char* msg,uint32_t size) {
  for (Task* task=headTask;task!=NULL;task=task->next) {
    if (task->pid==pid) {
      char* data=kmalloc(size);
      for (int i=0;i<size;i++) {
        data[i]=msg[i];
      }
      task->msg_store[task->wr]=data;
      task->sender_store[task->wr]=currentTask->pid;
      task->size_store[task->wr]=size;
      task->wr++;
      if (task->wr==16) {
        task->wr=0;
      }
      if (task->wr==task->rd) {
        task->wr--;
        if (task->wr==255) {
          task->wr=15;
        }
      }
      break;
    }
  }
}

void* tasking_get_msg(uint32_t* sender,uint32_t* size) {
  if (!currentTask->msg_store) {
    return NULL;
  }
  if (currentTask->msg_store[currentTask->rd]==NULL) {
    currentTask->rd++;
    if (currentTask->rd==16) {
      currentTask->rd=0;
    }
    if (currentTask->msg_store[currentTask->rd]==NULL) {
      currentTask->rd--;
      if (currentTask->rd==255) {
        currentTask->rd=15;
      }
      return NULL;
    }
  }
  *sender=currentTask->sender_store[currentTask->rd];
  *size=currentTask->size_store[currentTask->rd];
  char* data=currentTask->msg_store[currentTask->rd];
  char* msg=alloc_pages((*size/4096)+1);
  for (int i=0;i<*size;i++) {
    msg[i]=data[i];
  }
  kfree(data);
  currentTask->msg_store[currentTask->rd]=NULL;
  currentTask->sender_store[currentTask->rd]=0;
  currentTask->rd++;
  if (currentTask->rd==16) {
    currentTask->rd=0;
  }
  if (currentTask->rd>currentTask->wr) {
    currentTask->rd=currentTask->wr-1;
    if (currentTask->rd==255) {
      currentTask->rd=15;
    }
  }
  return msg;
}

void tasking_yield(registers_t registers) {
  Task* task=currentTask->next;
  if (!task) {
    task=headTask;
  }
  load_smap(task->cr3);
  switch_to_task(task);
}
