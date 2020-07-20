#include "tasking.h"
#include "../tasking.h"
#include <sys/types.h>
#include "kmalloc.h"
#include "serial.h"
#include "../halt.h"
#include "paging.h"
#include "tasking_helpers.h"
#define MAX_PROCS 32768
#define HAS_UNBLOCKED_THREADS(proc) (proc->numThreads!=proc->numThreadsBlocked)
#define NUM_UNBLOCKED_THREADS(proc) (proc->numThreads-proc->numThreadsBlocked)
#define SAME_PROC(thread1,thread2) (thread1->process->pid==thread2->process->pid)
#define SAME_THREAD(thread1,thread2) (thread1->process->pid==thread2->process->pid&&thread1->tid==thread2->tid)
uint32_t next_pid=0;
uint32_t num_procs=0;
Process* processes[MAX_PROCS];
char proc_schedule_bmap[MAX_PROCS/8];
Thread* currentThread;
static Thread* readyToRunHead=NULL;
static Thread* readyToRunTail=NULL;
static uint32_t* kstacks=(uint32_t*)0xC8000000;

static char is_proc_scheduled(uint32_t index) {
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  char entry=proc_schedule_bmap[byte];
  return (entry&(1<<bit))>0;
}

static void mark_proc_scheduled(uint32_t index) {
  if (is_proc_scheduled(index)) {
    serial_printf("Attempt to schedule a thread in a process with a scheduled thread! (PID %d)\n",index);
    halt();
  }
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  proc_schedule_bmap[byte]=proc_schedule_bmap[byte]|(1<<bit);
}

static void unmark_proc_scheduled(uint32_t index) {
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  proc_schedule_bmap[byte]=proc_schedule_bmap[byte]&(~(1<<bit));
}

void tasking_createTask(void* eip,void* cr3,char kmode,char param1_exists,uint32_t param1_arg,char param2_exists,uint32_t param2_arg,char isThread) {
  if (next_pid>MAX_PROCS && !isThread) {
    serial_printf("Failed to create a process, as 32k processes have been created already.\n");
    halt(); //Cannot ever create more than 32k processes, as I don't currently reuse PIDs.
  }
  uint32_t param1;
  if (param1_exists) {
    param1=param1_arg;
  } else {
    param1=1;
  }
  uint32_t param2;
  if (param2_exists) {
    if (isThread) {
      serial_printf("Param2 in Thread!\n");
      halt();
    }
    param2=param2_arg;
  } else {
    param2=2;
  }
  Process* proc;
  Thread* thread=kmalloc(sizeof(Thread));
  if (isThread) {
    proc=processes[(pid_t)param2_arg];
    proc->numThreads++;
    thread->cr3=proc->firstThread->cr3;
  } else {
    proc=kmalloc(sizeof(Process));
    if (currentThread) {
      proc->priv=currentThread->process->priv;
    } else {
      proc->priv=1;
    }
    proc->pid=next_pid;
    next_pid++;
    proc->next_tid=0;
    proc->numThreads=1;
    proc->numThreadsBlocked=0;
    proc->firstThread=thread;
    processes[proc->pid]=proc;
    thread->cr3=cr3;
  }
  thread->process=proc;
  thread->errno=0;
  thread->tid=proc->next_tid;
  proc->next_tid++;
  void* old_cr3;
  asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(old_cr3)::"%eax");
  uint32_t kstack_num=new_kstack();
  load_address_space((uint32_t)thread->cr3);
  if (kmode) {
    uint32_t top_idx=(1024*(kstack_num+1));
    thread->kernel_esp=((uint32_t)(&kstacks[top_idx-5]));
    thread->kernel_esp_top=thread->kernel_esp;
    kstacks[top_idx-1]=(uint32_t)eip;
  } else {
    uint32_t top_idx=(1024*(kstack_num+1));
    thread->kernel_esp=((uint32_t)(&kstacks[top_idx-7]));
    thread->kernel_esp_top=thread->kernel_esp;
    kstacks[top_idx-3]=(uint32_t)task_init;
    uint32_t* user_stack=(uint32_t*)(((uint32_t)alloc_pages(2))+0x2000);
    int buffer_pg_num=(((uint32_t)user_stack)-0x2000)>>12;
    make_protector(buffer_pg_num);
    user_stack-=2;
    user_stack[0]=param1;
    user_stack[1]=param2;
    kstacks[top_idx-2]=(uint32_t)user_stack;
    kstacks[top_idx-1]=(uint32_t)eip;
  }
  load_address_space((uint32_t)old_cr3);
  thread->prevReadyToRun=NULL;
  thread->nextReadyToRun=NULL;
  if (isThread) {
    thread->nextThreadInProcess=proc->firstThread;
    thread->prevThreadInProcess=NULL;
    thread->state=THREAD_READY;
    proc->firstThread->prevThreadInProcess=thread;
    proc->firstThread=thread;
  } else {
    thread->nextThreadInProcess=NULL;
    thread->prevThreadInProcess=NULL;
    if (!is_proc_scheduled(proc->pid)) {
      if (readyToRunTail) {
        thread->state=THREAD_READY;
        readyToRunTail->nextReadyToRun=thread;
        thread->prevReadyToRun=readyToRunTail;
        readyToRunTail=thread;
        mark_proc_scheduled(proc->pid);
      } else if (currentThread) {
        thread->state=THREAD_READY;
        readyToRunHead=thread;
        readyToRunTail=thread;
        mark_proc_scheduled(proc->pid);
      } else {
        thread->state=THREAD_RUNNING;
        currentThread=thread;
      }
    }
  }
  if (!isThread) {
    num_procs++;
  }
  serial_printf("Created thread with PID %d and TID %d.\n",proc->pid,thread->tid);
}

void tasking_init() {
  void* cr3;
  asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(cr3)::"%eax");
  tasking_createTask(NULL,cr3,1,0,0,0,0,0);
}

char tasking_isPrivleged() {
  return currentThread->process->priv;
}

pid_t getPID() {
  return currentThread->process->pid;
}

int* tasking_get_errno_address() {
  return &currentThread->errno;
}

void tasking_new_thread(void* start) {
  tasking_createTask(start,NULL,0,0,0,0,getPID(),1);
}

void switch_to_thread(Thread* thread) {
  // Unlink the thread from the list of ready-to-run threads
  if (thread!=readyToRunHead) {
    thread->prevReadyToRun->nextReadyToRun=thread->nextReadyToRun;
    if (thread->nextReadyToRun) {
      thread->nextReadyToRun->prevReadyToRun=thread->prevReadyToRun;
    }
  } else {
    readyToRunHead=thread->nextReadyToRun;
    if (readyToRunHead==NULL) {
      readyToRunTail=NULL;
    }
  }
  unmark_proc_scheduled(thread->process->pid);
  thread->prevReadyToRun=NULL;
  thread->nextReadyToRun=NULL;
  if (currentThread->state==THREAD_RUNNING) {
    currentThread->state=THREAD_READY;
  }
  Thread* currentThreadNextReady=currentThread->nextThreadInProcess;
  while ((currentThreadNextReady&&currentThreadNextReady->state!=THREAD_READY)||(currentThreadNextReady&&SAME_THREAD(thread,currentThreadNextReady))) {
    currentThreadNextReady=currentThreadNextReady->nextThreadInProcess;
  }
  if (!currentThreadNextReady) {
    currentThreadNextReady=currentThread->process->firstThread;
    while ((currentThreadNextReady&&currentThreadNextReady->state!=THREAD_READY)||(currentThreadNextReady&&SAME_THREAD(thread,currentThreadNextReady))) {
      currentThreadNextReady=currentThreadNextReady->nextThreadInProcess;
    }
  }
  if (!currentThreadNextReady) {
    //This process is fully blocked, try the process of the thread we're yielding to
    currentThreadNextReady=thread->nextThreadInProcess;
    while ((currentThreadNextReady&&currentThreadNextReady->state!=THREAD_READY)||(currentThreadNextReady&&SAME_THREAD(thread,currentThreadNextReady))) {
      currentThreadNextReady=currentThreadNextReady->nextThreadInProcess;
    }
    if (!currentThreadNextReady) {
      currentThreadNextReady=thread->process->firstThread;
      while ((currentThreadNextReady&&currentThreadNextReady->state!=THREAD_READY)||(currentThreadNextReady&&SAME_THREAD(thread,currentThreadNextReady))) {
        currentThreadNextReady=currentThreadNextReady->nextThreadInProcess;
      }
    }
  }
  if (currentThreadNextReady && !is_proc_scheduled(currentThread->process->pid)) {
    // Link the thread onto the list of ready to run threads
    if (readyToRunTail) {
      currentThreadNextReady->prevReadyToRun=readyToRunTail;
      readyToRunTail->nextThreadInProcess=currentThreadNextReady;
      readyToRunTail=currentThreadNextReady;
    } else {
      readyToRunHead=currentThreadNextReady;
      readyToRunTail=currentThreadNextReady;
    }
    mark_proc_scheduled(currentThread->process->pid);
  }
  serial_printf("Switching to PID %d TID %d.\n",thread->process->pid,thread->tid);
  load_smap((uint32_t)thread->cr3);
  switch_to_thread_asm(thread);
}

void tasking_yield() {
  serial_printf("Attempting to yield\n");
  if (readyToRunHead) {
    serial_printf("Attempting to switch to PID %d TID %d\n",readyToRunHead->process->pid,readyToRunHead->tid);
    switch_to_thread(readyToRunHead);
  } else {
    if (NUM_UNBLOCKED_THREADS(currentThread->process)>1) {
      serial_printf("The ready to run list is empty, and the current process has other unblocked threads? This is an invalid state! Halting!\n");
      halt();
    } else if (NUM_UNBLOCKED_THREADS(currentThread->process)==1) {
      serial_printf("Yield failed, no other ready processes\n");
      return;
    } else {
      if (num_procs==0) {
        serial_printf("All processes exited, halting\n");
        halt();
      } else {
        serial_printf("All threads in all processes blocked, waiting for an IRQ which unblocks a thread\n");
        // All threads in all processes blocked, so wait for an IRQ whose handler unblocks a thread.
        do {
          asm volatile("sti"); //As interrupts are stopped, re-enable them.
          asm volatile("hlt"); //Wait for an interrupt handler to run and return.
          asm volatile("cli"); //Clear interrupts, as tasking code must not be interrupted.
        } while (readyToRunHead==NULL);
      }
      serial_printf("Attempting to switch to PID %d TID %d\n",readyToRunHead->process->pid,readyToRunHead->tid);
      switch_to_thread(readyToRunHead);
    }
  }
}

void tasking_block(ThreadState newstate) {
  if (readyToRunHead&&SAME_THREAD(readyToRunHead,currentThread)) {
    readyToRunHead=readyToRunHead->nextReadyToRun;
    if (readyToRunHead==NULL) {
      readyToRunTail=NULL;
    }
  }
  if (readyToRunTail&&SAME_THREAD(readyToRunTail,currentThread)) {
    readyToRunTail=readyToRunTail->prevReadyToRun;
    if (readyToRunTail==NULL) {
      readyToRunHead=NULL;
    }
  }
  if (readyToRunHead&&readyToRunHead->nextReadyToRun) {
    for (Thread* thread=readyToRunHead->nextReadyToRun;thread!=NULL;thread=thread->nextReadyToRun) {
      if (SAME_THREAD(thread,currentThread)) {
        thread->prevReadyToRun->nextReadyToRun=thread->nextReadyToRun;
        if (thread->nextReadyToRun) {
          thread->nextReadyToRun->prevReadyToRun=thread->prevReadyToRun;
        }
        break;
      }
    }
  }
  for (Thread* thread=currentThread->process->firstThread;thread!=NULL;thread=thread->nextThreadInProcess) {
    if (thread->tid==currentThread->tid) {
      thread->state=newstate;
    }
  }
}
void tasking_unblock(pid_t pid,uint32_t tid) {
    serial_printf("Unblocking PID %d TID %d\n",pid,tid);
    if (!processes[pid]) {
      serial_printf("PID %d does not exist!\n",pid);
    }
    Thread* thread=processes[pid]->firstThread;
    for (;thread!=NULL;thread=thread->nextThreadInProcess) {
      if (thread->tid==tid) {
        break;
      }
    }
    if (!thread) {
      serial_printf("PID %d TID %d does not exist!\n",pid,thread);
    }
    if (thread->tid!=tid) {
      serial_printf("Error! Got wrong thread! (Wanted TID %d, got TID %d)\n",tid,thread->tid);
      halt();
    }
    thread->state=THREAD_READY;
    if (!is_proc_scheduled(thread->process->pid)) {
    // Link the thread onto the list of ready to run threads
    if (readyToRunTail) {
      thread->prevReadyToRun=readyToRunTail;
      readyToRunTail->nextThreadInProcess=thread;
      readyToRunTail=thread;
    } else {
      readyToRunHead=thread;
      readyToRunTail=thread;
    }
    mark_proc_scheduled(thread->process->pid);
  }
}

void tasking_exit(uint8_t code) {
  serial_printf("PID %d is exiting with code %d.\n",currentThread->process->pid,code);
  if (readyToRunHead&&SAME_PROC(readyToRunHead,currentThread)) {
    readyToRunHead=readyToRunHead->nextReadyToRun;
    if (readyToRunHead==NULL) {
      readyToRunTail=NULL;
    }
  }
  if (readyToRunTail&&SAME_PROC(readyToRunTail,currentThread)) {
    readyToRunTail=readyToRunTail->prevReadyToRun;
    if (readyToRunTail==NULL) {
      readyToRunHead=NULL;
    }
  }
  if (readyToRunHead&&readyToRunHead->nextReadyToRun) {
    for (Thread* thread=readyToRunHead->nextReadyToRun;thread!=NULL;thread=thread->nextReadyToRun) {
      if (SAME_PROC(thread,currentThread)) {
        thread->prevReadyToRun->nextReadyToRun=thread->nextReadyToRun;
        if (thread->nextReadyToRun) {
          thread->nextReadyToRun->prevReadyToRun=thread->prevReadyToRun;
        }
        break;
      }
    }
  }
  for (Thread* thread=currentThread->process->firstThread;thread!=NULL;thread=thread->nextThreadInProcess) {
    thread->state=THREAD_EXITED;
  }
  currentThread->process->numThreadsBlocked=currentThread->process->numThreads;
  num_procs--;
  tasking_yield();
}
