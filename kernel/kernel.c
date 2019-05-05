#include "../cpu/cpu_init.h"
#include "../drivers/vga.h"
// #include "../drivers/pci.h"
// #include "../drivers/serial.h"
// #include "../cpu/i386/ports.h"
// #include "vfs.h"
// #include "../fs/devfs.h"
// #include "../fs/initrd.h"
#include <grub/text_fb_info.h>
#include <stdlib.h>
// #include <tasking.h>
#include <string.h>
#include <memory.h>
#include <grub/multiboot2.h>
// #include "klog.h"
#include "elf.h"
// #include <errno.h>
// #include "../drivers/ide.h"
// #include "parts.h"
#include <stdint.h>

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


void kmain(struct multiboot_boot_header_tag* hdr) {
  tags=hdr;
  cpu_init(tags);
  text_fb_info info;
  info.address=(char*)0xC00B8000;
  info.width=80;
  info.height=25;
  vga_init(info);
  read_initrd(tags);
  int pos=0;
  uint32_t datapos;
  for (uint32_t i=0;i<1;i++) {
      uint32_t name_size;
      char* name_sz_ptr=(char*)&name_size;
      name_sz_ptr[0]=initrd[pos];
      name_sz_ptr[1]=initrd[pos+1];
      name_sz_ptr[2]=initrd[pos+2];
      name_sz_ptr[3]=initrd[pos+3];
      pos+=4;
      if (name_size==0) {
        break;
      }
      if (strcmp("init",&initrd[pos])==0) {
        pos+=5;
        uint32_t contents_size;
        char* cont_sz_ptr=(char*)&contents_size;
        cont_sz_ptr[0]=initrd[pos];
        cont_sz_ptr[1]=initrd[pos+1];
        cont_sz_ptr[2]=initrd[pos+2];
        cont_sz_ptr[3]=initrd[pos+3];
        pos+=4;
        datapos=pos;
        pos+=contents_size;
        break;
      }
      uint32_t contents_size;
      char* cont_sz_ptr=(char*)&contents_size;
      cont_sz_ptr[0]=initrd[pos];
      cont_sz_ptr[1]=initrd[pos+1];
      cont_sz_ptr[2]=initrd[pos+2];
      cont_sz_ptr[3]=initrd[pos+3];
      pos+=4;
      pos+=contents_size;
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
    for (int i=0;i<header.pheader_ent_nm;i++) {
      elf_pheader pheader;
      pos=(header.prog_hdr)+(header.pheader_ent_sz*i)+datapos;
      char* phdr_ptr=(char*)&pheader;
      for (size_t i=0;i<sizeof(elf_pheader);i++) {
        phdr_ptr[i]=initrd[pos];
        pos++;
      }
      alloc_memory_virt(((pheader.memsz)/4096)+1,(void*)pheader.vaddr);
      memset((void*)pheader.vaddr,0,pheader.memsz);
      if (pheader.filesz>0) {
        pos=pheader.offset+datapos;
        char* data_ptr=(char*)pheader.vaddr;
        for (size_t i=0;i<pheader.filesz;i++) {
          data_ptr[i]=initrd[pos];
          pos++;
        }
      }
    }
    func_ptr prog=(func_ptr)header.entry;
    prog();
  }
}
