#include "cpu/halt.h"
#include "cpu/paging.h"
#include "cpu/serial.h"
#include "cpu/tasking_helpers.h"
#include "kmalloc.h"
#include "tasking.h"
#include <sys/types.h>
#include <string.h>
#include "rpc.h"

#define MAX_PROCS 32768 //!< Maximum number of processes that can be running at a time
#define HAS_UNBLOCKED_THREADS(proc) (proc->num_threads!=proc->num_threads_blocked) //!< Macro to check whethe a process has unblocked threads
#define NUM_UNBLOCKED_THREADS(proc) (proc->num_threads-proc->num_threads_blocked) //!< Macro to get the number of unblocked threads for a process
#define SAME_PROC(thread1,thread2) (thread1->process->pid==thread2->process->pid) //!< Macro to check whether two threads have the same PID
pid_t next_pid=0; //!< PID to use for the next created process
size_t num_procs=0; //!< Number of non-exited processes
Process processes[MAX_PROCS]; //!< Array pf processes by PID
char proc_schedule_bmap[MAX_PROCS/8]; //!< Bitmap of what processes are scheduled
Thread* current_thread; //!< Currently running thread
static Thread* ready_to_run_head=NULL; //!< Head of the linked list of ready to run threads
static Thread* ready_to_run_tail=NULL; //!< Tail of the linked list of ready to run threads
static Thread* thread_to_be_freed; //!< Thread that exited and needs to be freed

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

/**
 * Schedules a thread if the thread's prcess does not have a scheduled thread
 * \param thread The thread to schedule
 */
void schedule_thread(Thread* thread) {
  if(!is_proc_scheduled(thread->process->pid)) {
    if (ready_to_run_head) {
      thread->state=THREAD_READY;
      // ready_to_run_tail->next_ready_to_run=thread;
      // thread->prev_ready_to_run=ready_to_run_tail;
      // ready_to_run_tail=thread;
      if (thread==ready_to_run_head) {
        serial_printf("TASKING DATA STRUCTURES CORRUPT!!!!\n");
        halt();
      }
      thread->next_ready_to_run=ready_to_run_head;
      ready_to_run_head->prev_ready_to_run=thread;
      ready_to_run_head=thread;
      mark_proc_scheduled(thread->process->pid);
    } else if (current_thread) {
      thread->state=THREAD_READY;
      ready_to_run_head=thread;
      ready_to_run_tail=thread;
      mark_proc_scheduled(thread->process->pid);
    } else {
      thread->state=THREAD_RUNNING;
      current_thread=thread;
    }
  }
}

void tasking_create_task(void* eip,void* address_space,char kmode,void* param1,void* param2,char isThread,char is_irq_handler) {
  if (next_pid>MAX_PROCS && !isThread) {
    serial_printf("Failed to create a process, as 32k processes have been created already.\n");
    halt(); //Cannot ever create more than 32k processes, as I don't currently reuse PIDs.
  }
  pid_t pid=isThread ? (pid_t)param2 : next_pid++;
  Process* proc=&processes[pid];
  Thread* thread=kmalloc(sizeof(Thread));
  proc->num_threads++;
  thread->process=proc;
  thread->errno=0;
  thread->tid=proc->next_tid++;
  thread->prev_ready_to_run=NULL;
  thread->next_ready_to_run=NULL;
  thread->prev_thread_in_process=NULL;
  thread->state=THREAD_READY;
  if (isThread) {
    thread->address_space=proc->first_thread->address_space;
    thread->next_thread_in_process=proc->first_thread;
    proc->first_thread->prev_thread_in_process=thread;
  } else {
    thread->address_space=address_space;
    thread->next_thread_in_process=NULL;
    proc->priv=current_thread ? current_thread->process->priv : 1;
    proc->pid=pid;
    num_procs++;
  }
  proc->first_thread=thread;
  setup_kstack(thread,param1,param2,kmode,eip,is_irq_handler);
  schedule_thread(thread);
  /* serial_printf("Created thread with PID %d and TID %d.\n",proc->pid,thread->tid); */
  /* serial_printf("Structure values:\n"); */
  /* serial_printf("kernel_esp=%x\n",thread->kernel_esp); */
  /* serial_printf("kernel_esp_top=%x\n",thread->kernel_esp_top); */
  /* serial_printf("address_space=%x\n",thread->address_space); */
  /* serial_printf("tid=%d\n",thread->tid); */
  /* serial_printf("state=%d\n",thread->state); */
  /* serial_printf("next_thread_in_process=%x\n",thread->next_thread_in_process); */
  /* serial_printf("next_ready_to_run=%x\n",thread->next_ready_to_run); */
  /* serial_printf("prev_ready_to_run=%x\n",thread->prev_ready_to_run); */
  /* serial_printf("process=%x\n",thread->process); */
}

void tasking_init() {
  for (size_t i = 0; i < MAX_PROCS; i++) {
    memset(&processes[i],0,sizeof(Process));
  }

  tasking_create_task(NULL,get_address_space(),1,NULL,NULL,0,0);
}

char tasking_is_privleged() {
  return current_thread->process->priv;
}

pid_t tasking_get_PID() {
  return current_thread->process->pid;
}

pid_t tasking_get_TID() {
  return current_thread->tid;
}

int* tasking_get_errno_address() {
  return &current_thread->errno;
}

pid_t tasking_new_thread(void* start,pid_t pid,void* param,char is_irq_handler) {
  tasking_create_task(start,NULL,0,param,(void*)pid,1,is_irq_handler);
  return processes[pid].first_thread->tid;
}

/**
 * Get the next ready thread in a list of threads, starting at the specified thread's next thread
 * \param thread The start thread
 * \param thread_to_skip A thread to skip even if it's ready
 * \return the next ready thread
 */
static Thread* get_next_ready_thread(Thread* thread,Thread* thread_to_skip) {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define HELPER \
  while (thread&&(thread->state!=THREAD_READY||thread==thread_to_skip)) { \
    thread=thread->next_thread_in_process; \
  }
  //end define
#endif
  Thread* start_of_list=thread->process->first_thread;
  thread=thread->next_thread_in_process;
  HELPER;
  if (!thread) {
    thread=start_of_list;
    HELPER;
  }
  return thread;
#undef HELPER
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
  //Get the next ready thread in the current process
  Thread* current_thread_next_ready=get_next_ready_thread(current_thread,thread);
  if (!current_thread_next_ready) {
    //This process is fully blocked, try the process of the thread we're yielding to
    current_thread_next_ready=get_next_ready_thread(thread,thread);
  }
  if (current_thread_next_ready) {  
    schedule_thread(current_thread_next_ready);
  }
  thread->state=THREAD_RUNNING;
  serial_printf("Switching to PID %d TID %d.\n",thread->process->pid,thread->tid);
  switch_to_thread_asm(thread);
  if (thread_to_be_freed) {
    serial_printf("Freeing PID %d TID %d.\n",thread_to_be_freed->process->pid,thread_to_be_freed->tid);
    if (thread_to_be_freed->prev_thread_in_process) {
      thread_to_be_freed->prev_thread_in_process->next_thread_in_process = thread_to_be_freed->next_thread_in_process;
    }
    if (thread_to_be_freed->next_thread_in_process) {
      thread_to_be_freed->next_thread_in_process->prev_thread_in_process = thread_to_be_freed->prev_thread_in_process;
    }
    free_kstack(thread_to_be_freed->kernel_esp);
    kfree(thread_to_be_freed);
  }
}

void tasking_yield() {
  if (ready_to_run_head) {
    serial_printf("Attempting to switch to PID %d TID %d\n",ready_to_run_head->process->pid,ready_to_run_head->tid);
    switch_to_thread(ready_to_run_head);
  } else {
    if (NUM_UNBLOCKED_THREADS(current_thread->process)>1) {
      // Thread* thread=get_next_ready_thread(current_thread,current_thread);
      // schedule_thread(thread);
      // yield();
      serial_printf("The ready to run list is empty, and the current process has other unblocked threads? This is an invalid state! Halting!\n");
      halt();
    } else if (NUM_UNBLOCKED_THREADS(current_thread->process)==1) {
      return;
    } else {
      if (num_procs==0) {
        serial_printf("All processes exited, halting\n");
        asm volatile("cli");
        for(;;);
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
  if (ready_to_run_head==current_thread) {
    ready_to_run_head=ready_to_run_head->next_ready_to_run;
    if (ready_to_run_head==NULL) {
      ready_to_run_tail=NULL;
    }
  }
  if (ready_to_run_tail==current_thread) {
    ready_to_run_tail=ready_to_run_tail->prev_ready_to_run;
    if (ready_to_run_tail==NULL) {
      ready_to_run_head=NULL;
    }
  }
  if (ready_to_run_head&&ready_to_run_head->next_ready_to_run) {
    for (Thread* thread=ready_to_run_head->next_ready_to_run;thread!=NULL;thread=thread->next_ready_to_run) {
      if (thread==current_thread) {
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
  current_thread->process->num_threads_blocked++;
  if (current_thread->process->num_threads_blocked==current_thread->process->num_threads) {
    unmark_proc_scheduled(current_thread->process->pid);
  }
  tasking_yield();
}

/**
 * Get a thread 
 * \param pid The PID of the thread
 * \param tid The TID of the thread
 * \return the thread wih the specified PID and TID
 */
static Thread* get_thread(pid_t pid,pid_t tid) {
  if (processes[pid].num_threads==0) {
    serial_printf("PID %d does not exist!\n",pid);
    return NULL;
  }
  Thread* thread=processes[pid].first_thread;
  for (;thread!=NULL;thread=thread->next_thread_in_process) {
    if (thread->tid==tid) {
      break;
    }
  }
  if (!thread) {
    serial_printf("PID %d TID %d does not exist!\n",pid,thread);
    return NULL;
  }
  if (thread->tid!=tid) {
    serial_printf("Error! Got wrong thread! (Wanted TID %d, got TID %d)\n",tid,thread->tid);
    return NULL;
  }
  return thread;
}

void tasking_unblock(pid_t pid,pid_t tid) {
  serial_printf("Unblocking PID %d TID %d\n",pid,tid);
  Thread* thread=get_thread(pid,tid);
  if (thread==NULL) {
    return;
  }
  if (thread->state==THREAD_EXITED||thread->state==THREAD_READY||thread->state==THREAD_RUNNING) {
    serial_printf("Tried to unblock an exited/ready/running thread!\n");
    return;
  }
  thread->state=THREAD_READY;
  thread->process->num_threads_blocked--;
  schedule_thread(thread);
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
    if (thread->state!=THREAD_EXITED) {
      thread->state=THREAD_EXITED;
      if (thread==current_thread->process->first_thread && kernel_rpc_is_init(current_thread->process->pid)) {
        continue;
      }
      if (thread!=current_thread) {
        free_kstack(thread->kernel_esp);
        kfree(thread);
      } else {
        thread_to_be_freed = current_thread;
      }
    }
  }
  current_thread->process->num_threads_blocked=current_thread->process->num_threads;
  num_procs--;
  tasking_yield();
}

void* tasking_get_address_space(pid_t pid) {
  return processes[pid].first_thread->address_space;
}

void tasking_set_rpc_calling_thread(pid_t pid,pid_t tid) {
  Thread* thread=get_thread(pid,tid);
  thread->rpc_calling_pid=current_thread->process->pid;
  thread->rpc_calling_tid=current_thread->tid;
}

pid_t tasking_get_rpc_calling_thread(pid_t* tid) {
  *tid=current_thread->rpc_calling_tid;
  return current_thread->rpc_calling_pid;
} 

void tasking_set_rpc_ret_buf(void* buf) {
  pid_t tid;
  pid_t pid=tasking_get_rpc_calling_thread(&tid);
  Thread* thread=get_thread(pid,tid);
  thread->rpc_ret_buf=buf;
}

void* tasking_get_rpc_ret_buf() {
  return current_thread->rpc_ret_buf;
}

void tasking_thread_exit() {
  serial_printf("PID %d TID %d is exiting\n",current_thread->process->pid,current_thread->tid);
  tasking_block(THREAD_EXITED);
  thread_to_be_freed=current_thread;
}

char tasking_check_proc_exists(pid_t pid) {
  if (processes[pid].num_threads==0) {
    return 0;
  }
  char num_exited_threads=0;
  for (Thread* thread=processes[pid].first_thread;thread!=NULL;thread=thread->next_thread_in_process) {
    if (thread->state==THREAD_EXITED) {
      num_exited_threads++;
    }
  }
  if ((num_exited_threads=processes[pid].num_threads)&&kernel_get_num_rpc_funcs(pid)==0) {
    return 0;
  } else {
    return 1;
  }
}
