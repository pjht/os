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
#include "elf.h"

#include <stdint.h>
#define VIRT_OFFSET 0xC0000000

uint32_t total_mb;
uint32_t mem_map[MMAP_ENTRIES+1][2];

char* initrd=NULL;
uint32_t initrd_sz;

devbuf* kbd_buf;

void switch_to_user_mode() {
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
    pop %eax; \
    or $0x200,%eax; \
    push %eax; \
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

void main(multiboot_info_t* mbd) {
  cpu_init();
  init_vfs();
  init_devfs();
  read_initrd(mbd);
  add_dev(initrd_dev_drv,"initrd");
  init_initrd();
  mount("/","/dev/initrd","initrd");
  screen_init();
  kbd_buf=devbuf_init();
  add_dev(console_dev_drv,"console");
  stdout=fopen("/dev/console","w");
  stdin=fopen("/dev/console","r");
  stderr=fopen("/dev/console","w");
  ps2_init();
  serial_init();
  parallel_init();
  if (!stdout) {
    FILE* serial0=fopen("/dev/ttyS0","w");
    if (serial0) {
      stdout=serial0;
      stderr=serial0;
    }
  }
  timer_init();
  klog("INFO","Waiting for 1 second");
  wait(1000);
  pci_init();
  tasking_init();
  // uint32_t pid=fork();
  // if (pid==0) {
  //   while (1) {
  //     printf("Child\n");
  //     yield();
  //   }
  // } else {
  //   while (1) {
  //     printf("Parent. Child PID:%d\n",pid);
  //     yield();
  //   }
  // }
  printf(">");
  while (1) {
    char str[256];
    fgets(str,256,stdin);
    str[strlen(str)-1]='\0';
    char* cmd=strtok(str," ");
    if (strcmp(cmd,"cat")==0) {
      char* file=strtok(NULL," ");
      FILE* fd=fopen(file,"r");
      if (fd!=NULL) {
        for (char c=fgetc(fd);c!=EOF;c=fgetc(fd)) {
          putc(c);
        }
      }
    }
    if (strcmp(cmd,"mount")==0) {
      char* dev=strtok(NULL," ");
      char* mntpnt=strtok(NULL," ");
      char* type=strtok(NULL," ");
      mount(mntpnt,dev,type);
      free(mntpnt);
      free(dev);
      free(type);
    }
    if (strcmp(cmd,"pci")==0) {
      char* subcmd=strtok(NULL," ");
      if (strcmp(subcmd,"list")==0) {
        for (int i=0;i<pci_num_devs;i++) {
          pci_dev_common_info* info=pci_devs[i];
          printf("PCI device %d:\n  Main class:",i);
          switch (info->class_code) {
            case PCI_CLASS_UNCLASSIFIED:
              printf("Unclassified\n");
              break;
            case PCI_CLASS_STORAGE:
              printf("Storage\n");
              break;
            case PCI_CLASS_NETWORK:
              printf("Network\n");
              break;
            case PCI_CLASS_DISPLAY:
              printf("Display\n");
              break;
            case PCI_CLASS_MULTIMEDIA:
              printf("Multimedia\n");
              break;
            case PCI_CLASS_MEMORY:
              printf("Memory\n");
              break;
            case PCI_CLASS_BRIDGE:
              printf("Bridge\n");
              break;
            case PCI_CLASS_SIMPCOM:
              printf("Simpcom\n");
              break;
            case PCI_CLASS_BASEPERIPH:
              printf("Baseperiph\n");
              break;
            case PCI_CLASS_INPDEV:
              printf("Inpdev\n");
              break;
            case PCI_CLASS_DOCK:
              printf("Dock\n");
              break;
            case PCI_CLASS_CPU:
              printf("Cpu\n");
              break;
            case PCI_CLASS_SERBUS:
              printf("Serbus\n");
              break;
            case PCI_CLASS_WIRELESS:
              printf("Wireless\n");
              break;
            case PCI_CLASS_INTELLIGENT:
              printf("Intelligent\n");
              break;
            case PCI_CLASS_SATELLITE:
              printf("Satellite\n");
              break;
            case PCI_CLASS_ENCRYPTION:
              printf("Encryption\n");
              break;
            case PCI_CLASS_SIGPROCESS:
              printf("Sigprocess\n");
              break;
          }
        }
      }
      if (strcmp(subcmd,"cfgdmp")==0) {
        char* devnumstr=strtok(NULL," ");
        devnumstr=strrev(devnumstr);
        uint32_t devnum=0;
        uint32_t x=1;
        for (int i=0;i<strlen(devnumstr);i++) {
          devnum+=(devnumstr[i]-0x30)*x;
          x=x*10;
        }
        free(devnumstr);
        uint8_t* info=(uint8_t*)pci_devs[devnum];
        for (int i=0;i<16;i++) {
          printf("%x ",info[i]);
        }
        printf("\n");
      }
      free(subcmd);
    }
    if (strcmp(cmd,"rdelf")==0) {
      FILE* file=fopen(strtok(NULL," "),"r");
      elf_header* header=malloc(sizeof(elf_header));
      fread(header,sizeof(elf_header),1,file);
      fseek(file,header->prog_hdr,SEEK_SET);
      elf_pheader** p_ents=malloc(sizeof(elf_pheader*)*header->pheader_ent_nm);
      for (int i=0;i<header->pheader_ent_nm;i++) {
        p_ents[i]=malloc(sizeof(elf_pheader));
        fread(p_ents[i],sizeof(elf_pheader),1,file);
      }
      for (int i=0;i<header->pheader_ent_nm;i++) {
        if (p_ents[i]->type!=1) {
          continue;
        }
        printf("Copy %d bytes starting at %x in the file to the virtual address %x\n",p_ents[i]->filesz,p_ents[i]->offset,p_ents[i]->vaddr);
        char* mem=alloc_memory(1);
        for (size_t j=0;j<p_ents[i]->memsz;j++) {
          mem[i]=0;
        }
        fseek(file,p_ents[i]->offset,SEEK_SET);
        fread(mem,1,p_ents[i]->filesz,file);
        alloc_pages(p_ents[i]->vaddr,virt_to_phys(mem),1,1,1,page_directory,page_tables);
        int (*start)()=header->entry;
        int ret=start();
        printf("Return val:%d",ret);
      }
    }
    free(cmd);
    printf(">");
  }
	switch_to_user_mode();
  for(;;);
}

void got_key(char key) {
  devbuf_add(key,kbd_buf);
}
