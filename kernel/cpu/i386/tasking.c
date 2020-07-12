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
#include "../halt.h"
#include "serial.h"
#include "../../vga_err.h"
#include <sys/types.h>
#define STACK_PAGES 2

extern void task_init();
static uint32_t* kstacks=(uint32_t*)0xF6800000;


uint32_t next_pid=0;
uint32_t running_blocked_tasks=0;
Task* currentTask=NULL;
static Task* readyToRunHead=NULL;
static Task* readyToRunTail=NULL;
static Task* exitedTasksHead=NULL;
static Task* exitedTasksTail=NULL;
static Task* tasks[32768];
Task* tasking_createTaskCr3KmodeParam(void* eip,void* cr3,char kmode,char param1_exists,uint32_t param1_arg,char param2_exists,uint32_t param2_arg);

void tasking_init(void* esp) {
  tasking_createTaskCr3KmodeParam(NULL,paging_new_address_space(),1,0,0,0,0);
}

Task* tasking_createTaskCr3KmodeParam(void* eip,void* cr3,char kmode,char param1_exists,uint32_t param1_arg,char param2_exists,uint32_t param2_arg) {
    if (next_pid>1024*32) {
      serial_printf("Failed to create a task, as 32k tasks have been created already.\n");
      halt(); //Cannot ever create more than 32k tasks, as I don't currently reuse PIDs.
    }
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
      kstacks[top_idx-1]=(uint32_t)eip;
    } else {
      uint32_t top_idx=(1024*(next_pid+1));
      task->kernel_esp=((uint32_t)(&kstacks[top_idx-7]));
      task->kernel_esp_top=task->kernel_esp;
      kstacks[top_idx-7]=0;
      kstacks[top_idx-6]=0;
      kstacks[top_idx-5]=0;
      kstacks[top_idx-4]=0;
      kstacks[top_idx-3]=(uint32_t)task_init;
      uint32_t* user_stack=(uint32_t*)(((uint32_t)alloc_pages(2))+0x2000);
      int buffer_pg_num=(((uint32_t)user_stack)-0x2000)>>12;
      make_protector(buffer_pg_num);
      // uint32_t* user_stack=(uint32_t*)(((uint32_t)alloc_pages(1))+0x1000);
      user_stack-=2;
      user_stack[0]=param1;
      user_stack[1]=param2;
      kstacks[top_idx-2]=(uint32_t)user_stack;
      kstacks[top_idx-1]=(uint32_t)eip;
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
    task->prev=NULL;
    task->next=NULL;
    if (readyToRunTail) {
      task->state=TASK_READY;
      readyToRunTail->next=task;
      task->prev=readyToRunTail;
      readyToRunTail=task;
    } else if (currentTask) {
      task->state=TASK_READY;
      readyToRunHead=task;
      readyToRunTail=task;
    } else {
      task->state=TASK_RUNNING;
      currentTask=task;
    }
    tasks[next_pid]=task;
    next_pid++;
    running_blocked_tasks++;
    if (task->pid!=0) {
      serial_printf("Created task with PID %d.\n",task->pid);
    }
    return task;
}

int* tasking_get_errno_address() {
  return &(currentTask->errno);
}
char isPrivleged(uint32_t pid) {
  for (Task* task=readyToRunHead;task!=NULL;task=task->next) {
    if (task->pid==pid) {
      return task->priv;
    }
  }
  return 0;
}

Task* tasking_createTask(void* eip) {
  return tasking_createTaskCr3KmodeParam(eip,paging_new_address_space(),0,0,0,0,0);
}

void switch_to_task(Task* task) {
  if (task!=readyToRunHead) {
    // Unlink it from the doubly-linked list of ready-to-run tasks
    task->prev->next=task->next;
    if (task->next) {
      task->next->prev=task->prev;
    }
  } else {
    // Unlink the task from the list by advancing the ready to run pointer to the next task
    readyToRunHead=task->next;
    // If the task did not have a next task, also clear the ready to run tail pointer
    if (readyToRunHead==NULL) {
      readyToRunTail=NULL;
    }
  }
  task->prev=NULL;
  task->next=NULL;
  task->state=TASK_RUNNING;
  if (currentTask->state==TASK_RUNNING) {
    currentTask->state=TASK_READY;
    // Link the task onto the list of ready to run tasks
    if (readyToRunTail) {
      currentTask->prev=readyToRunTail;
      readyToRunTail->next=currentTask;
      readyToRunTail=currentTask;
    } else {
      readyToRunHead=currentTask;
      readyToRunTail=currentTask;
    }
  }
  serial_printf("Yielding to PID %d.\n",task->pid);
  load_smap(task->cr3);
  switch_to_task_asm(task);
}

void tasking_yield() {
  serial_printf("Yield called from pid %d\n",currentTask->pid);
  if (!readyToRunHead) {
    if (currentTask->state!=TASK_RUNNING) {
      //This indicates either all tasks are bloked or the os has shutdown. Check which one it is
      if (running_blocked_tasks==0) {
        serial_printf("All tasks exited, halting\n");
        halt(); //never returns, so we dont need an else
      }
      serial_printf("All tasks blocked, waiting for interrupt to unblock task\n");
      // All tasks blocked
      // Stop running the current task by setting currentTask to null, though put it in a local variable to keep track of it.
      Task* task=currentTask;
      currentTask=NULL;
      // Wait for an IRQ whose handler unblocks a task
      do {
        asm volatile("sti");
        asm volatile("hlt");
        asm volatile("cli");
      } while (readyToRunHead==NULL);
      currentTask=task;
    } else {
      serial_printf("Yield failed, no other ready tasks\n");
      return;
    }
  }
  switch_to_task(readyToRunHead);
}

void tasking_yieldToPID(uint32_t pid) {
  Task* task=tasks[pid];
  if (!task) {
    serial_printf("PID %d does not exist.\n",pid);
    return;
  }
  switch_to_task(task);
}

void tasking_exit(uint8_t code) {
  serial_printf("PID %d is exiting with code %d.\n",currentTask->pid,code);
  currentTask->state=TASK_EXITED;
  if (exitedTasksHead) {
    exitedTasksTail->next=currentTask;
    currentTask->prev=exitedTasksHead;
    exitedTasksTail=currentTask;
  } else {
    exitedTasksHead=currentTask;
    exitedTasksTail=currentTask;
  }
  running_blocked_tasks--;
  tasking_yield();
}

uint32_t getPID() {
  return currentTask->pid;
}

void tasking_block(TaskState newstate) {
    if (newstate==TASK_RUNNING || newstate==TASK_READY || newstate==TASK_EXITED) {
      serial_printf("Attempted to block task using a new state of running/ready/exited. This is an error.\n");
      return;
    }
    serial_printf("Blocking PID %d with state %d\n",currentTask->pid,currentTask->state);
    currentTask->state = newstate;
    tasking_yield();
}

void tasking_unblock(pid_t pid) {
  serial_printf("Unblocking PID %d (task pid %d)\n",pid,tasks[pid]->pid);
    tasks[pid]->state=TASK_READY;
    if(readyToRunHead) {
      readyToRunTail->next=tasks[pid];
      readyToRunTail=tasks[pid];
    } else {
      readyToRunHead=tasks[pid];
      readyToRunTail=tasks[pid];
    }
}
