#include "../drivers/screen.h"
#include "../drivers/serial.h"
#include "../drivers/parallel.h"
#include "../drivers/ps2.h"
#include "../drivers/pci.h"
#include "../drivers/pci.h"
#include "../drivers/timer.h"
#include "../libc/string.h"
#include "../libc/stdlib.h"
#include "../libc/stdio.h"
#include "../libc/devbuf.h"
#include "../cpu/cpu_init.h"
#include "../cpu/halt.h"
#include "../cpu/syscalls.h"
#include "../cpu/tasking.h"
#include "../cpu/memory.h"
#include "../cpu/i386/paging.h"
#include "../fs/devfs.h"
#include "../fs/initrd.h"
#include "multiboot.h"
#include "kernel.h"
#include "vfs.h"
#include "klog.h"
#include "pppp.h"
#include "elf.h"

#include <stdint.h>
#define VIRT_OFFSET 0xC0000000

uint32_t total_mb;
uint32_t mem_map[MMAP_ENTRIES+1][2];

char* initrd=NULL;
uint32_t initrd_sz;

devbuf* kbd_buf;

/*
pop %eax; \
or $0x200,%eax; \
push %eax; \
*/
void switch_to_user_mode() {
   // Set up a stack structure for switching to user mode.
   asm volatile("  \
     cli; \
     mov $0x23, %ax; \
     mov %ax, %ds; \
     mov %ax, %es; \
     mov %ax, %fs; \
     mov %ax, %gs; \
                   \
     mov %esp, %eax; \
     pushl $0x23; \
     pushl %eax; \
     pushf; \
     pushl $0x1B; \
     push $1f; \
     iret; \
   1: \
     ");
}

void get_memory(multiboot_info_t* mbd) {
  if ((mbd->flags&MULTIBOOT_INFO_MEM_MAP)!=0) {
    uint32_t mmap_length=mbd->mmap_length;
    struct multiboot_mmap_entry* mmap_addr=(struct multiboot_mmap_entry*)(mbd->mmap_addr+VIRT_OFFSET);
    uint32_t size;
    struct multiboot_mmap_entry* mmap_entry=mmap_addr;
    int i;
    for (i=0;(uint32_t)mmap_entry<((uint32_t)mmap_addr+mmap_length);mmap_entry=(struct multiboot_mmap_entry*)((uint32_t)mmap_entry+size+4)) {
      if (i>=MMAP_ENTRIES) {
        break;
      }
      size=mmap_entry->size;
      uint32_t start_addr=mmap_entry->addr;
      uint32_t length=mmap_entry->len;
      uint32_t type=mmap_entry->type;
      if (type!=1) {
        continue;
      }
      mem_map[i][0]=start_addr;
      mem_map[i][1]=start_addr+length-1;
      i++;
    }
    mem_map[i][0]=0;
    mem_map[i][0]=0;
    total_mb=0;
  } else if ((mbd->flags&MULTIBOOT_INFO_MEMORY)!=0) {
    total_mb=((mbd->mem_upper)/1024)+2;
    mem_map[0][0]=0;
    mem_map[0][1]=0;
  } else {
    printf("PANIC: Cannot detect memory!");
    halt();
  }
}

void print_memory() {
  char str[100];
  if (total_mb>0) {
    if (total_mb%1024==0) {
      int_to_ascii(total_mb/1024,str);
    } else {
      int_to_ascii(total_mb,str);
    }
    printf(str);
    if (total_mb%1024==0) {
      printf(" GB ");
    } else {
      printf(" MB ");
      printf("of memory detected\n");
    }
  } else {
    for (int i=0;i<MMAP_ENTRIES;i++) {
      if (mem_map[i][0]==0&&mem_map[i][1]==0) {
        break;
      }
      printf("Memory from %x to %x\n",mem_map[i][0],mem_map[i][1]);
    }
  }
}

int initrd_dev_drv(char* filename,int c,long pos,char wr) {
  if (wr) {
    return 0;
  }
  if (pos>=initrd_sz) {
    return EOF;
  }
  return initrd[pos];
}

int console_dev_drv(char* filename,int c,long pos,char wr) {
  if (wr) {
    if (c=='\f') {
      screen_clear();
    } else if (c=='\b') {
      screen_backspace();
    }
    char str[2];
    str[0]=(char)c;
    str[1]='\0';
    screen_write_string(str);
    return 0;
  } else {
    return devbuf_get(kbd_buf);
  }
}

void read_initrd(multiboot_info_t* mbd) {
  if ((mbd->flags&MULTIBOOT_INFO_MODS)!=0) {
    uint32_t mods_count=mbd->mods_count;
    if (mods_count>0) {
      while (mods_count>0) {
        multiboot_module_t* mods_addr=(multiboot_module_t*)(mbd->mods_addr+VIRT_OFFSET);
        if (strcmp((char*)(mods_addr->cmdline+VIRT_OFFSET),"initrd")==0) {
          initrd=malloc(sizeof(char)*(mods_addr->mod_end-mods_addr->mod_start));
          initrd_sz=(mods_addr->mod_end-mods_addr->mod_start);
          memcpy(initrd,(void*)mods_addr->mod_start+VIRT_OFFSET,initrd_sz);
        };
        mods_count--;
      }
    }
  } else {
    halt();
  }
  if (!initrd) {
    halt();
  }
}

void init() {
  klog("INFO","Beginning initialization");
  //createTask(vfs_task);
  // while (vfs_initialized==0) {
  //   //yield();
  //   klog("INFO","NO_VFS");
  // }
  // klog("INFO","VFS");
  // init_devfs();
  // screen_init();
  // kbd_buf=devbuf_init();
  // add_dev(console_dev_drv,"console");
  stdout=fopen("/dev/console","w");
  // while (stdout==NO_FD) {
  //   yield();
  // }
  // klog("INFO","STDOUT");
  // stdin=fopen("/dev/console","r");
  // stderr=fopen("/dev/console","w");
  // yield();
  // send_msg(0,"Hello");
  // yield();
  // while (1) {
  //   uint32_t sender;
  //   char* msg=get_msg(&sender);
  //   if (msg) {
  //     // Handle message here
  //   }
  //   yield();
  // }
  while(1) {
    yield();
  }
}

void main(multiboot_info_t* mbd) {
  cpu_init();
  tasking_init();
  //switch_to_user_mode();
  createTask(init);
  while (1) {
    yield();
  }
}

void got_key(char key) {
  devbuf_add(key,kbd_buf);
}
