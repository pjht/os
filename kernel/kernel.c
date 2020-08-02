#include "cpu/cpu_init.h"
#include "cpu/isr.h"
#include "cpu/paging.h"
#include "cpu/serial.h"
#include "pmem.h"
#include "tasking.h"
#include "vga_err.h"
#include <elf.h>
#include <grub/multiboot2.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <tasking.h>
#include "timer/timer.h"

/**
 * REspresents a TAR file header
*/
typedef struct {
  char filename[100]; //!< Filename of file descried by the tar header
  char mode[8]; //!< Mode as an octal string
  char uid[8]; //!< UID of owner as an octal string
  char gid[8]; //!< GID of owner as an octal string
  char size[12]; //!< Size of file as an octal string
  char mtime[12]; //!< Modification time as an octal string
  char chksum[8]; //!< Checksum as octal string
  char typeflag[1]; //!< File type. (0 for normal file)
} tar_header;

long initrd_sz;
char* initrd;
typedef int (*func_ptr)();
static struct multiboot_boot_header_tag* tags;

static void read_initrd(struct multiboot_boot_header_tag* tags) {
  struct multiboot_tag* tag=(struct multiboot_tag*)(tags+1);
  while (tag->type!=0) {
    switch (tag->type) {
      case MULTIBOOT_TAG_TYPE_MODULE: {
        struct multiboot_tag_module* mod=(struct multiboot_tag_module*) tag;
        initrd=malloc(sizeof(char)*(mod->mod_end-mod->mod_start));
        initrd_sz=mod->mod_end-mod->mod_start;
        memcpy(initrd,(void*)(mod->mod_start+0xC0000000),mod->mod_end-mod->mod_start);
      }
    }
    tag=(struct multiboot_tag*)((char*)tag+((tag->size+7)&0xFFFFFFF8));
  }
}

size_t getsize(const char *in) {
    size_t size=0;
    size_t j;
    size_t count=1;
    for (j=11;j>0;j--,count*=8) {
        size+=((in[j-1]-'0')*count);
    }
    return size;
}

void kmain(struct multiboot_boot_header_tag* hdr) {
  tags=hdr;
  cpu_init();
  serial_init();
  pmem_init(tags);
  paging_init();
  isr_install();
  asm volatile("sti");
  tasking_init();
  vga_init((char*)0xC00B8000);
  timer_init(100);
  read_initrd(tags);
  int pos=0;
  size_t datapos;
  tar_header* tar_hdr;
  for (int i=0;;i++) {
    tar_hdr=(tar_header*)&initrd[pos];
    if (tar_hdr->filename[0]=='\0') break;
    size_t size=getsize(tar_hdr->size);
    pos+=512;
    if (strcmp(&tar_hdr->filename[0],"init")==0) {
      datapos=pos;
      break;
    }
    pos+=size;
    if (pos%512!=0) {
      pos+=512-(pos%512);
    }
  }
  elf_header header;
  pos=datapos;
  char* hdr_ptr=(char*)&header;
  for (size_t i=0;i<sizeof(elf_header);i++) {
    hdr_ptr[i]=initrd[pos];
    pos++;
  }
  if (header.magic!=ELF_MAGIC) {
    vga_write_string("[INFO] Invalid magic number for prog.elf\n");
  } else {
    void* address_space=new_address_space();
    for (int i=0;i<header.pheader_ent_nm;i++) {
      elf_pheader pheader;
      pos=(header.prog_hdr)+(header.pheader_ent_sz*i)+datapos;
      char* phdr_ptr=(char*)&pheader;
      for (size_t i=0;i<sizeof(elf_pheader);i++) {
        phdr_ptr[i]=initrd[pos];
        pos++;
      }
      char* ptr=alloc_memory(((pheader.memsz)/4096)+1);
      memset(ptr,0,pheader.memsz);
      if (pheader.filesz>0) {
        pos=pheader.offset+datapos;
        for (size_t i=0;i<pheader.filesz;i++) {
          ptr[i]=initrd[pos];
          pos++;
        }
      }
      copy_data(address_space,ptr,pheader.memsz,(void*)pheader.vaddr);
    }
    create_proc((void*)header.entry,address_space,NULL,NULL);
    // for (int i=0;i<4;i++) {
    //   yield();
    // }
    // unblock_thread(1,0);
    // for (int i=0;i<4;i++) {
    //   yield();
    // }
    // for (;;);
    exit(0);
  }
}
