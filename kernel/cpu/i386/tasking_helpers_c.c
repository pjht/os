#include "../../tasking.h"
#include "../paging.h"
#include "../tasking_helpers.h"
#include "../../pmem.h"

static uint32_t* kstacks=(uint32_t*)0xC8000000;
static uint32_t kstack_bmap[(218*1024)/8]={0};

static char get_bmap_bit(uint32_t index) {
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  char entry=kstack_bmap[byte];
  return (entry&(1<<bit))>0;
}

static void set_bmap_bit(uint32_t index) {
  uint32_t byte=index/8;
  uint32_t bit=index%8;
  kstack_bmap[byte]=kstack_bmap[byte]|(1<<bit);
}

int new_kstack() {
  int num=-1;
  for (int i=0;i<(218*1024);i++) {
    if (get_bmap_bit(i)==0) {
      num=i;
      break;
    }
  }
  if (num==-1) {
    return -1;
  }
  set_bmap_bit(num);
  map_pages(((char*)kstacks+num*0x1000),pmem_alloc(1),1,1,1);
  return num;
}

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
