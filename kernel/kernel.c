#include <grub/text_fb_info.h>
#include <stdlib.h>
#include <tasking.h>
#include <string.h>
#include <memory.h>
#include <grub/multiboot2.h>
#include <stdint.h>
#include "cpu/cpu_init.h"
#include "vga_err.h"
#include <elf.h>

typedef struct {
  char filename[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag[1];
} tar_header;

static long initrd_sz;
static char* initrd;
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

uint32_t getsize(const char *in) {
    uint32_t size=0;
    uint32_t j;
    uint32_t count=1;
    for (j=11;j>0;j--,count*=8) {
        size+=((in[j-1]-'0')*count);
    }
    return size;
}

void kmain(struct multiboot_boot_header_tag* hdr) {
  tags=hdr;
  cpu_init(tags);
  vga_init((char*)0xC00B8000);
  read_initrd(tags);
  int pos=0;
  uint32_t datapos;
  tar_header* tar_hdr;
  for (int i=0;;i++) {
    tar_hdr=(tar_header*)&initrd[pos];
    if (tar_hdr->filename[0]=='\0') break;
    uint32_t size=getsize(tar_hdr->size);
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
    void* cr3=new_address_space();
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
      copy_data(cr3,ptr,pheader.memsz,(void*)pheader.vaddr);
    }
    char* initrd2=alloc_memory((initrd_sz/4096)+1);
    memcpy(initrd2,initrd,initrd_sz);
    initrd2=put_data(cr3,initrd2,initrd_sz);
    createTaskCr3Param((void*)header.entry,cr3,(uint32_t)initrd2,initrd_sz);
    for(;;) {
      yield();
    }
  }
}
