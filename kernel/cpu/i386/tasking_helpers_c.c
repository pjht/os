#include "../../tasking.h"
#include "../paging.h"
#include "../tasking_helpers.h"

static uint32_t* kstacks=(uint32_t*)0xC8000000;

void setup_kstack(Thread* thread,uint32_t param1,uint32_t param2,char kmode,void* eip) {
  void* old_cr3=get_cr3();
  uint32_t kstack_num=new_kstack();
  load_address_space(thread->cr3);
  if (kmode) {
    uint32_t top_idx=(1024*(kstack_num+1));
    thread->kernel_esp=((uint32_t)(&kstacks[top_idx-5]));
    thread->kernel_esp_top=thread->kernel_esp;
    kstacks[top_idx-1]=(uint32_t)eip;
  } else {
    uint32_t top_idx=(1024*(kstack_num+1));
    thread->kernel_esp=((uint32_t)(&kstacks[top_idx-7]));
    thread->kernel_esp_top=thread->kernel_esp;
    uint32_t* user_stack=(uint32_t*)(((uint32_t)alloc_pages(2))+0x2000);
    user_stack-=2;
    user_stack[0]=param1;
    user_stack[1]=param2;
    kstacks[top_idx-3]=(uint32_t)task_init;
    kstacks[top_idx-2]=(uint32_t)user_stack;
    kstacks[top_idx-1]=(uint32_t)eip;
  }
  load_address_space(old_cr3);
}
