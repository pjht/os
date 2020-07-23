#include "cpu/halt.h"
#include "cpu/paging.h"
#include "cpu/serial.h"
#include "cpu/tasking_helpers.h"
#include "kmalloc.h"
#include "tasking.h"
#include <sys/types.h>

#define MAX_PROCS 32768
#define HAS_UNBLOCKED_THREADS(proc) (proc->numThreads!=proc->numThreadsBlocked)
#define NUM_UNBLOCKED_THREADS(proc) (proc->numThreads-proc->numThreadsBlocked)
#define SAME_PROC(thread1,thread2) (thread1->process->pid==thread2->process->pid)
#define SAME_THREAD(thread1,thread2) (thread1->process->pid==thread2->process->pid&&thread1->tid==thread2->tid)
pid_t next_pid=0;
size_t num_procs=0;
Process* processes[MAX_PROCS];
char proc_schedule_bmap[MAX_PROCS/8];
Thread* currentThread;
static Thread* readyToRunHead=NULL;
static Thread* readyToRunTail=NULL;

static char is_proc_scheduled(pid_t index) {
  size_t byte=index/8;
  size_t bit=index%8;
  char entry=proc_schedule_bmap[byte];
  return (entry&(1<<bit))>0;
}

static void mark_proc_scheduled(pid_t index) {
  if (is_proc_scheduled(index)) {
    serial_printf("Attempt to schedule a thread in a process with a scheduled thread! (PID %d)\n",index);
    halt();
  }
  size_t byte=index/8;
  size_t bit=index%8;
  proc_schedule_bmap[byte]=proc_schedule_bmap[byte]|(1<<bit);
}

static void unmark_proc_scheduled(pid_t index) {
  size_t byte=index/8;
  size_t bit=index%8;
  proc_schedule_bmap[byte]=proc_schedule_bmap[byte]&(~(1<<bit));
}

void tasking_createTask(void* eip,void* cr3,char kmode,char param1_exists,void* param1_arg,char param2_exists,void* param2_arg,char isThread) {
  if (next_pid>MAX_PROCS && !isThread) {
    serial_printf("Failed to create a process, as 32k processes have been created already.\n");
    halt(); //Cannot ever create more than 32k processes, as I don't currently reuse PIDs.
  }
  void* param1;
  if (param1_exists) {
    param1=param1_arg;
  } else {
    param1=NULL;
  }
  void* param2;
  if (param2_exists) {
    if (isThread) {
      serial_printf("Param2 in Thread!\n");
      halt();
    }
    param2=param2_arg;
  } else {
    param2=NULL;
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
  setup_kstack(thread,param1,param2,kmode,eip);
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
  tasking_createTask(NULL,get_cr3(),1,0,0,0,0,0);
}

char tasking_isPrivleged() {
  return currentThread->process->priv;
}

pid_t tasking_getPID() {
  return currentThread->process->pid;
}

int* tasking_get_errno_address() {
  return &currentThread->errno;
}

pid_t tasking_new_thread(void* start,pid_t pid,char param_exists,void* param_arg) {
  tasking_createTask(start,NULL,0,param_exists,param_arg,0,(void*)pid,1);
  return processes[pid]->firstThread->tid;
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
        do { wait_for_unblocked_thread_asm(); } while (readyToRunHead==NULL);
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
void tasking_unblock(pid_t pid,pid_t tid) {
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

void tasking_exit(int code) {
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
  unmark_proc_scheduled(currentThread->process->pid);
  for (Thread* thread=currentThread->process->firstThread;thread!=NULL;thread=thread->nextThreadInProcess) {
    thread->state=THREAD_EXITED;
  }
  currentThread->process->numThreadsBlocked=currentThread->process->numThreads;
  num_procs--;
  tasking_yield();
}
