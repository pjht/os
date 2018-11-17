#include "paging.h"
#include "paging_helpers.h"
#include "isr.h"
#include "../../libc/stdlib.h"
#include <stdint.h>
#define STACK_PAGES 2

typedef struct Task {
    uint32_t eax,ebx,ecx,edx;
    uint32_t esi,edi,esp,ebp;
    uint32_t eip,eflags,cr3;
    struct Task *next;
    uint32_t pid;
} Task;

Task* tasks=(Task*)0;
Task* current_task=(Task*)0;
uint32_t next_pid=0;
char disable_switching=0;

// Task* new_task(char* prg_start,uint32_t prg_size,unsigned int eip,unsigned int esp) {
//   Task* task=kmalloc(sizeof(Task*));
//   uint32_t* page_dir=kmalloc(sizeof(uint32_t)*1024);
//   uint32_t* kern_page_tables=kmalloc(sizeof(uint32_t)*1024*NUM_KERN_DIRS);
//   alloc_pages((void*)KERN_VIRT_START,(void*)KERN_PHYS_START,NUM_KERN_DIRS*1024,0,1,page_dir,kern_page_tables);
//   double prg_pages_double=prg_size/4096;
//   int prg_pages=(int)prg_pages_double;
//   if (prg_pages_double!=(double)prg_pages) {
//     prg_pages++;
//   }
//   uint32_t* prg_page_tables=kmalloc(sizeof(uint32_t)*1024*prg_pages+STACK_PAGES);
//   alloc_pages(0x0,virt_to_phys(prg_start),prg_pages+STACK_PAGES,1,1,page_dir,prg_page_tables);
//   task->eip=eip;
//   task->esp=(uint32_t)prg_start+prg_size;
//   task->cr3=((uint32_t)page_dir)-0xC0000000;
//   if (tasks) {
//       tasks->next=task;
//   }
//   task->next=(Task*)0;
//   task->pid=next_pid;
//   next_pid++;
//   tasks=task;
//   return task;
// }

void fork() {

}

void init_tasking() {
  Task* task=malloc(sizeof(Task*));
  task->cr3=((uint32_t)page_directory)-0xC0000000;
  task->next=(Task*)0;
  task->pid=next_pid;
  next_pid++;
  tasks=task;
  current_task=task;
}

void switch_tasks(registers_t regs) {
  Task* next=current_task->next;
  if (next==0) {
    next=tasks;
  }
  current_task->eax=regs.eax;
  current_task->ebx=regs.ebx;
  current_task->ecx=regs.ecx;
  current_task->edx=regs.edx;
  current_task->esi=regs.esi;
  current_task->edi=regs.edi;
  current_task->ebp=regs.ebp;
  current_task->esp=regs.esp;
  current_task->eip=regs.eip;
  current_task->eflags=regs.eflags;
  set_regs(next->eax,next->ebx,next->ecx,next->edx,next->esi,next->edi,next->esp,next->ebp,next->eip,next->eflags);
}
