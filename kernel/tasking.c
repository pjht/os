/**
 * \file 
*/

#include "cpu/halt.h"
#include "cpu/paging.h"
#include "cpu/serial.h"
#include "cpu/tasking_helpers.h"
#include "kmalloc.h"
#include "tasking.h"
#include <sys/types.h>

#define MAX_PROCS 32768 //!< Maximum number of processes that can be running at a time
#define HAS_UNBLOCKED_THREADS(proc) (proc->num_threads!=proc->num_threads_blocked) //!< Macro to check whethe a process has unblocked threads
#define NUM_UNBLOCKED_THREADS(proc) (proc->num_threads-proc->num_threads_blocked) //!< Macro to get the number of unblocked threads for a process
#define SAME_PROC(thread1,thread2) (thread1->process->pid==thread2->process->pid) //!< Macro to check whether two threads have the same PID
#define SAME_THREAD(thread1,thread2) (thread1->process->pid==thread2->process->pid&&thread1->tid==thread2->tid) //!< Macro to check whether two threads have the same PID and TID
pid_t next_pid=0; //!< PID to use for the next created process
size_t num_procs=0; //!< Number of non-exited processes
Process processes[MAX_PROCS]; //!< Array pf processes by PID
char proc_schedule_bmap[MAX_PROCS/8]; //!< Bitmap of what processes are scheduled
Thread* current_thread; //!< Currently running thread
static Thread* ready_to_run_head=NULL; //!< Head of the linked list of ready to run threads
static Thread* ready_to_run_tail=NULL; //!< Tail of the linked list of ready to run threads

/**
 * Check whether a process is scheduled
 * \param index The PID to check
 * \return whether the process is scheduled
*/
static char is_proc_scheduled(pid_t index) {
  size_t byte=index/8;
  size_t bit=index%8;
  char entry=proc_schedule_bmap[byte];
  return (entry&(1<<bit))>0;
}

/**
 * Mark a process as scheduled
 * \param index The PID to mark
*/
static void mark_proc_scheduled(pid_t index) {
  if (is_proc_scheduled(index)) {
    serial_printf("Attempt to schedule a thread in a process with a scheduled thread! (PID %d)\n",index);
    halt();
  }
  size_t byte=index/8;
  size_t bit=index%8;
  proc_schedule_bmap[byte]=proc_schedule_bmap[byte]|(1<<bit);
}

/**
 * Unmark a process as scheduled
 * \param index The PID to unmark
*/
static void unmark_proc_scheduled(pid_t index) {
  size_t byte=index/8;
  size_t bit=index%8;
  proc_schedule_bmap[byte]=proc_schedule_bmap[byte]&(~(1<<bit));
}

void tasking_create_task(void* eip,void* address_space,char kmode,char param1_exists,void* param1_arg,char param2_exists,void* param2_arg,char isThread) {
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
  Process* proc=&processes[(pid_t)param2_arg];
  Thread* thread=kmalloc(sizeof(Thread));
  if (isThread) {
    proc->num_threads++;
    thread->address_space=proc->first_thread->address_space;
  } else {
    proc=kmalloc(sizeof(Process));
    if (current_thread) {
      proc->priv=current_thread->process->priv;
    } else {
      proc->priv=1;
    }
    proc->pid=next_pid;
    next_pid++;
    proc->next_tid=0;
    proc->num_threads=1;
    proc->num_threads_blocked=0;
    proc->first_thread=thread;
    thread->address_space=address_space;
  }
  thread->process=proc;
  thread->errno=0;
  thread->tid=proc->next_tid;
  proc->next_tid++;
  setup_kstack(thread,param1,param2,kmode,eip);
  thread->prev_ready_to_run=NULL;
  thread->next_ready_to_run=NULL;
  if (isThread) {
    thread->next_thread_in_process=proc->first_thread;
    thread->prev_thread_in_process=NULL;
    thread->state=THREAD_READY;
    proc->first_thread->prev_thread_in_process=thread;
    proc->first_thread=thread;
  } else {
    thread->next_thread_in_process=NULL;
    thread->prev_thread_in_process=NULL;
    if (!is_proc_scheduled(proc->pid)) {
      if (ready_to_run_tail) {
        thread->state=THREAD_READY;
        ready_to_run_tail->next_ready_to_run=thread;
        thread->prev_ready_to_run=ready_to_run_tail;
        ready_to_run_tail=thread;
        mark_proc_scheduled(proc->pid);
      } else if (current_thread) {
        thread->state=THREAD_READY;
        ready_to_run_head=thread;
        ready_to_run_tail=thread;
        mark_proc_scheduled(proc->pid);
      } else {
        thread->state=THREAD_RUNNING;
        current_thread=thread;
      }
    }
  }
  if (!isThread) {
    num_procs++;
  }
  serial_printf("Created thread with PID %d and TID %d.\n",proc->pid,thread->tid);
}

void tasking_init() {
  for (size_t i = 0; i < MAX_PROCS; i++) {
    processes[i].num_threads=0;
  }
  
  tasking_create_task(NULL,get_address_space(),1,0,0,0,0,0);
}

char tasking_is_privleged() {
  return current_thread->process->priv;
}

pid_t tasking_get_PID() {
  return current_thread->process->pid;
}

int* tasking_get_errno_address() {
  return &current_thread->errno;
}

pid_t tasking_new_thread(void* start,pid_t pid,char param_exists,void* param_arg) {
  tasking_create_task(start,NULL,0,param_exists,param_arg,0,(void*)pid,1);
  return processes[pid].first_thread->tid;
}

/**
 * Switch to a thread and schedule the next ready thread in the current process, if there is one.
 * \param thread The thread to switch to
*/
void switch_to_thread(Thread* thread) {
  // Unlink the thread from the list of ready-to-run threads
  if (thread!=ready_to_run_head) {
    thread->prev_ready_to_run->next_ready_to_run=thread->next_ready_to_run;
    if (thread->next_ready_to_run) {
      thread->next_ready_to_run->prev_ready_to_run=thread->prev_ready_to_run;
    }
  } else {
    ready_to_run_head=thread->next_ready_to_run;
    if (ready_to_run_head==NULL) {
      ready_to_run_tail=NULL;
    }
  }
  unmark_proc_scheduled(thread->process->pid);
  thread->prev_ready_to_run=NULL;
  thread->next_ready_to_run=NULL;
  if (current_thread->state==THREAD_RUNNING) {
    current_thread->state=THREAD_READY;
  }
  Thread* current_thread_next_ready=current_thread->next_thread_in_process;
  while ((current_thread_next_ready&&current_thread_next_ready->state!=THREAD_READY)||(current_thread_next_ready&&SAME_THREAD(thread,current_thread_next_ready))) {
    current_thread_next_ready=current_thread_next_ready->next_thread_in_process;
  }
  if (!current_thread_next_ready) {
    current_thread_next_ready=current_thread->process->first_thread;
    while ((current_thread_next_ready&&current_thread_next_ready->state!=THREAD_READY)||(current_thread_next_ready&&SAME_THREAD(thread,current_thread_next_ready))) {
      current_thread_next_ready=current_thread_next_ready->next_thread_in_process;
    }
  }
  if (!current_thread_next_ready) {
    //This process is fully blocked, try the process of the thread we're yielding to
    current_thread_next_ready=thread->next_thread_in_process;
    while ((current_thread_next_ready&&current_thread_next_ready->state!=THREAD_READY)||(current_thread_next_ready&&SAME_THREAD(thread,current_thread_next_ready))) {
      current_thread_next_ready=current_thread_next_ready->next_thread_in_process;
    }
    if (!current_thread_next_ready) {
      current_thread_next_ready=thread->process->first_thread;
      while ((current_thread_next_ready&&current_thread_next_ready->state!=THREAD_READY)||(current_thread_next_ready&&SAME_THREAD(thread,current_thread_next_ready))) {
        current_thread_next_ready=current_thread_next_ready->next_thread_in_process;
      }
    }
  }
  if (current_thread_next_ready && !is_proc_scheduled(current_thread->process->pid)) {
    // Link the thread onto the list of ready to run threads
    if (ready_to_run_tail) {
      current_thread_next_ready->prev_ready_to_run=ready_to_run_tail;
      ready_to_run_tail->next_ready_to_run=current_thread_next_ready;
      ready_to_run_tail=current_thread_next_ready;
    } else {
      ready_to_run_head=current_thread_next_ready;
      ready_to_run_tail=current_thread_next_ready;
    }
    mark_proc_scheduled(current_thread->process->pid);
  }
  serial_printf("Switching to PID %d TID %d.\n",thread->process->pid,thread->tid);
  switch_to_thread_asm(thread);
}

void tasking_yield() {
  serial_printf("Attempting to yield\n");
  if (ready_to_run_head) {
    serial_printf("Attempting to switch to PID %d TID %d\n",ready_to_run_head->process->pid,ready_to_run_head->tid);
    switch_to_thread(ready_to_run_head);
  } else {
    if (NUM_UNBLOCKED_THREADS(current_thread->process)>1) {
      serial_printf("The ready to run list is empty, and the current process has other unblocked threads? This is an invalid state! Halting!\n");
      halt();
    } else if (NUM_UNBLOCKED_THREADS(current_thread->process)==1) {
      serial_printf("Yield failed, no other ready processes\n");
      return;
    } else {
      if (num_procs==0) {
        serial_printf("All processes exited, halting\n");
        halt();
      } else {
        serial_printf("All threads in all processes blocked, waiting for an IRQ which unblocks a thread\n");
        // All threads in all processes blocked, so wait for an IRQ whose handler unblocks a thread.
        do { wait_for_unblocked_thread_asm(); } while (ready_to_run_head==NULL);
      }
      serial_printf("Attempting to switch to PID %d TID %d\n",ready_to_run_head->process->pid,ready_to_run_head->tid);
      switch_to_thread(ready_to_run_head);
    }
  }
}

void tasking_block(thread_state newstate) {
  if (ready_to_run_head&&SAME_THREAD(ready_to_run_head,current_thread)) {
    ready_to_run_head=ready_to_run_head->next_ready_to_run;
    if (ready_to_run_head==NULL) {
      ready_to_run_tail=NULL;
    }
  }
  if (ready_to_run_tail&&SAME_THREAD(ready_to_run_tail,current_thread)) {
    ready_to_run_tail=ready_to_run_tail->prev_ready_to_run;
    if (ready_to_run_tail==NULL) {
      ready_to_run_head=NULL;
    }
  }
  if (ready_to_run_head&&ready_to_run_head->next_ready_to_run) {
    for (Thread* thread=ready_to_run_head->next_ready_to_run;thread!=NULL;thread=thread->next_ready_to_run) {
      if (SAME_THREAD(thread,current_thread)) {
        thread->prev_ready_to_run->next_ready_to_run=thread->next_ready_to_run;
        if (thread->next_ready_to_run) {
          thread->next_ready_to_run->prev_ready_to_run=thread->prev_ready_to_run;
        }
        break;
      }
    }
  }
  for (Thread* thread=current_thread->process->first_thread;thread!=NULL;thread=thread->next_thread_in_process) {
    if (thread->tid==current_thread->tid) {
      thread->state=newstate;
    }
  }
}
void tasking_unblock(pid_t pid,pid_t tid) {
    serial_printf("Unblocking PID %d TID %d\n",pid,tid);
    if (processes[pid].num_threads==0) {
      serial_printf("PID %d does not exist!\n",pid);
    }
    Thread* thread=processes[pid].first_thread;
    for (;thread!=NULL;thread=thread->next_thread_in_process) {
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
    if (thread->state==THREAD_EXITED||thread->state==THREAD_READY||thread->state==THREAD_RUNNING) {
      serial_printf("Tried to unblock an exited/ready/running thread!\n");
      return;
    }
    thread->state=THREAD_READY;
    if (!is_proc_scheduled(thread->process->pid)) {
    // Link the thread onto the list of ready to run threads
    if (ready_to_run_tail) {
      thread->prev_ready_to_run=ready_to_run_tail;
      ready_to_run_tail->next_thread_in_process=thread;
      ready_to_run_tail=thread;
    } else {
      ready_to_run_head=thread;
      ready_to_run_tail=thread;
    }
    mark_proc_scheduled(thread->process->pid);
  }
}

void tasking_exit(int code) {
  serial_printf("PID %d is exiting with code %d.\n",current_thread->process->pid,code);
  if (ready_to_run_head&&SAME_PROC(ready_to_run_head,current_thread)) {
    ready_to_run_head=ready_to_run_head->next_ready_to_run;
    if (ready_to_run_head==NULL) {
      ready_to_run_tail=NULL;
    }
  }
  if (ready_to_run_tail&&SAME_PROC(ready_to_run_tail,current_thread)) {
    ready_to_run_tail=ready_to_run_tail->prev_ready_to_run;
    if (ready_to_run_tail==NULL) {
      ready_to_run_head=NULL;
      
    }
  }
  if (ready_to_run_head&&ready_to_run_head->next_ready_to_run) {
    for (Thread* thread=ready_to_run_head->next_ready_to_run;thread!=NULL;thread=thread->next_ready_to_run) {
      if (SAME_PROC(thread,current_thread)) {
        thread->prev_ready_to_run->next_ready_to_run=thread->next_ready_to_run;
        if (thread->next_ready_to_run) {
          thread->next_ready_to_run->prev_ready_to_run=thread->prev_ready_to_run;
        }
        break;
      }
    }
  }
  unmark_proc_scheduled(current_thread->process->pid);
  for (Thread* thread=current_thread->process->first_thread;thread!=NULL;thread=thread->next_thread_in_process) {
    thread->state=THREAD_EXITED;
  }
  current_thread->process->num_threads_blocked=current_thread->process->num_threads;
  num_procs--;
  tasking_yield();
}
