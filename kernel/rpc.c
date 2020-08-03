/**
 * \file
*/

#include <sys/types.h>
#include <stddef.h>
#include "tasking.h"
#include "rpc.h"
#include "pmem.h"
#include "cpu/paging.h"
#include "cpu/arch_consts.h"
#include "kmalloc.h"
#include <string.h>
#include "cpu/serial.h"

/**
 * Represents a thread waiting for a process to finish RPC init
*/
typedef struct rpc_waiting_thread {
  pid_t waiting_pid; //!< The PID of the thread that is waiting for RPC init
  pid_t waiting_tid; //!< The TID of the thread that is waiting for RPC init
  pid_t process_waiting_on; //!< The process being waited on
  struct rpc_waiting_thread* next; //!< The next entry in the linked list
} rpc_waiting_thread;

rpc_func_info* process_funcs[32768]={NULL}; //!< Pointers to a list of registered functions for each process
size_t process_num_funcs[32768]={0}; //!< The number of functions each process has registered
char process_ready_bmap[32768/8]={0}; //!< A bitmap of processes that have completed RPC init
rpc_waiting_thread* waiting_thread_list=NULL; //!< A linked list of threads waiting for a process to finish RPC init

/**
 * Mark a process as ready to accept RPC calls
 * \param pid The pid to mark
*/
static void mark_init(pid_t pid) {
  size_t byte=pid/8;
  size_t bit=pid%8;
  process_ready_bmap[byte]=process_ready_bmap[byte]|(1<<bit);
}

/**
 * Mark a process as not ready to accept RPC calls
 * \param pid The pid to mark
*/
static void clear_init(pid_t pid) {
  size_t byte=pid/8;
  size_t bit=pid%8;
  process_ready_bmap[byte]=process_ready_bmap[byte]&(~(1<<bit));
}


/**
 * Check if a process is ready to accept RPC calls
 * \param pid The pid to check
 * \return whether the process is ready
*/
static char is_init(pid_t pid) {
  size_t byte=pid/8;
  size_t bit=pid%8;
  char entry=process_ready_bmap[byte];
  return (entry&(1<<bit))>0;
}

void* kernel_rpc_call(pid_t pid,char* name,void* buf,size_t size) {
  if (is_init(pid)==0) {
    rpc_waiting_thread* waiting_thread=kmalloc(sizeof(rpc_waiting_thread));
    if (waiting_thread==NULL) {
      serial_printf("Kmalloc unable to allocate waiting_thread\n");
      halt();
    }
    waiting_thread->process_waiting_on=pid;
    waiting_thread->waiting_pid=tasking_get_PID();
    waiting_thread->waiting_tid=tasking_get_TID();
    waiting_thread->next=waiting_thread_list;
    waiting_thread_list=waiting_thread;
    tasking_block(THREAD_WAITING_FOR_RPC_INIT);
  }
  rpc_func_info* func=NULL;
  rpc_func_info* funcs=process_funcs[pid];
  for (size_t i = 0; i < process_num_funcs[pid]; i++){
    if (strcmp(funcs[i].name,name)==0) {
      func=&funcs[i];
      break;
    }
  }
  if (func==NULL) {
    serial_printf("No function %s for PID %d\n",name,pid);
    return NULL;
  }
  void* virtaddr=NULL;
  if (buf) {
    virtaddr=alloc_pages((size/PAGE_SZ)+1);
    void* physaddr=virt_to_phys(virtaddr);
    memcpy(virtaddr,buf,size);
    unmap_pages(virtaddr,(size/PAGE_SZ)+1);
    RUN_IN_ADDRESS_SPACE(tasking_get_address_space(pid),{
      virtaddr=find_free_pages((size/PAGE_SZ)+1);
      map_pages(virtaddr,physaddr,(size/PAGE_SZ)+1,1,1);
    });
  }
  pid_t tid=tasking_new_thread(func->code,pid,virtaddr);
  tasking_set_rpc_calling_thread(pid,tid);
  // Block the thread and wait for an unblock from rpc_return
  tasking_block(THREAD_WAITING_FOR_RPC);
  // Now that RPC call has returned, pass the return buffer back to the caller
  return tasking_get_rpc_ret_buf();
}

void kernel_rpc_register_func(char* name,rpc_func code) {
  pid_t pid=tasking_get_PID();
  if (process_funcs[pid]==NULL) {
    process_funcs[pid]=kmalloc(sizeof(rpc_func_info)*32);
  }
  if (process_num_funcs[pid]==32) {
    serial_printf("Already registered 32 functions!");
    return;
  }
  rpc_func_info* info=&process_funcs[pid][process_num_funcs[pid]];
  strcpy(&info->name[0],name);
  info->code=code;
  process_num_funcs[pid]++;
}

void kernel_rpc_deallocate_buf(void* buf,size_t size) {
  if (buf==NULL) return;
  dealloc_pages((size/PAGE_SZ)+1,buf);
}

void kernel_rpc_return(void* buf,size_t size) {
  pid_t tid;
  pid_t pid=tasking_get_rpc_calling_thread(&tid);
  void* virtaddr=NULL;
  if (buf) {
    virtaddr=alloc_pages((size/PAGE_SZ)+1);
    void* physaddr=virt_to_phys(virtaddr);
    memcpy(virtaddr,buf,size);
    unmap_pages(virtaddr,(size/PAGE_SZ)+1);
    RUN_IN_ADDRESS_SPACE(tasking_get_address_space(pid),{
      virtaddr=find_free_pages((size/PAGE_SZ)+1);
      map_pages(virtaddr,physaddr,(size/PAGE_SZ)+1,1,1);
    });
  }
  tasking_set_rpc_ret_buf(virtaddr);
  tasking_unblock(pid,tid);
}

size_t kernel_get_num_rpc_funcs(pid_t pid) {
  return process_num_funcs[pid];
}

void kernel_rpc_mark_as_init() {
  mark_init(tasking_get_PID());
  if (waiting_thread_list) {
    rpc_waiting_thread* prev=NULL;
    for (rpc_waiting_thread* waiting_thread=waiting_thread_list;waiting_thread!=NULL;waiting_thread=waiting_thread->next) {
      if (waiting_thread->process_waiting_on==tasking_get_PID()) {
        tasking_unblock(waiting_thread->waiting_pid,waiting_thread->waiting_tid);
        if (waiting_thread==waiting_thread_list) {
          waiting_thread_list=waiting_thread_list->next;
        } else {
          prev->next=waiting_thread->next;
        }
        prev=waiting_thread;
      }
    }
  }
}

