/**
 * \file 
*/

#include "../../tasking.h"
#include "../paging.h"
#include "../tasking_helpers.h"
#include "../../pmem.h"
#include <stddef.h>

static void** kstacks=(void*)0xC8000000; //!< Pointer to all the thread kernel stacks
static char kstack_bmap[(218*1024)/8]={0}; //!< Bitmap of what kernel stacks have been allocated

/**
 * Check whether a kernel stack is allocated
 * \param index The kernel stack to check
 * \return whether the kernel stack is allocated
*/
static char is_kstack_allocated(size_t index) {
  size_t byte=index/8;
  size_t bit=index%8;
  char entry=kstack_bmap[byte];
  return (entry&(1<<bit))>0;
}

/**
 * Mark that a kernel stack is allocated
 * \param index The kernel stack to mark
*/
static void mark_kstack_allocated(size_t index) {
  size_t byte=index/8;
  size_t bit=index%8;
  kstack_bmap[byte]=kstack_bmap[byte]|(1<<bit);
}

/**
 * Allocate a kernel stack for a thread
 * \return The number of the new kernel stack, or -1 if none are unallocated.
*/
static int new_kstack() {
  int num=-1;
  for (int i=0;i<(218*1024);i++) {
    if (is_kstack_allocated(i)==0) {
      num=i;
      break;
    }
  }
  if (num==-1) {
    return -1;
  }
  mark_kstack_allocated(num);
  map_pages(((char*)kstacks+num*0x1000),pmem_alloc(1),1,1,1);
  return num;
}

void setup_kstack(Thread* thread,void* param1,void* param2,char kmode,void* eip) {
  size_t kstack_num=new_kstack();
  if (kmode) {
    size_t top_idx=(1024*(kstack_num+1));
    thread->kernel_esp=((void*)(&kstacks[top_idx-5]));
    thread->kernel_esp_top=thread->kernel_esp;
    kstacks[top_idx-1]=(void*)eip;
  } else {
    size_t top_idx=(1024*(kstack_num+1));
    thread->kernel_esp=((void*)(&kstacks[top_idx-7]));
    thread->kernel_esp_top=thread->kernel_esp;
    void** user_stack;
    RUN_IN_ADDRESS_SPACE(thread->address_space,{
      user_stack=(void**)(((char*)alloc_pages(2))+0x2000);
      user_stack-=2;
      user_stack[0]=param1;
      user_stack[1]=param2;
    });
    kstacks[top_idx-3]=(void*)task_init;
    kstacks[top_idx-2]=(void*)user_stack;
    kstacks[top_idx-1]=(void*)eip;
  }
}
