#include "paging.h"
#include "tasking.h"
#include "../libc/memory.h"
#include <stdint.h>
#define STACK_PAGES 2

Task* new_task(char* prg_start,uint32_t prg_size,unsigned int eip,unsigned int esp) {
  Task* task=alloc_kern_pages(1,1);
  uint32_t* page_dir=alloc_kern_pages(1,1);
  uint32_t* kern_page_tables=alloc_kern_pages(NUM_KERN_DIRS,1);
  alloc_pages((void*)KERN_VIRT_START,(void*)KERN_PHYS_START,NUM_KERN_DIRS*1024,0,1,page_dir,kern_page_tables);
  double prg_pages_double=prg_size/4096;
  int prg_pages=(int)prg_pages_double;
  if (prg_pages_double!=(double)prg_pages) {
    prg_pages++;
  }
  uint32_t* prg_page_tables=alloc_kern_pages(prg_pages+STACK_PAGES,1);
  alloc_pages(0x0,virt_to_phys(prg_start),prg_pages+STACK_PAGES,1,1,page_dir,prg_page_tables);
  task->eip=eip;
  task->esp=(uint32_t)prg_start+prg_size;
  task->page_dir=page_dir;
  return task;
}
