#include "../cpu/cpu_init.h"
#include "../drivers/vga.h"
#include "../drivers/pci.h"
#include "../drivers/serial.h"
// #include "../cpu/i386/ports.h"
#include "vfs.h"
#include "../fs/devfs.h"
#include "../fs/initrd.h"
#include <grub/text_fb_info.h>
#include <stdlib.h>
#include <tasking.h>
#include <string.h>
#include <memory.h>
#include <grub/multiboot2.h>
#include "klog.h"
#include "elf.h"
#include <errno.h>
#include "../drivers/ide.h"
#include "parts.h"
#include "../fs/ext2.h"
#include <stdint.h>

static long initrd_sz;
static char* initrd;
typedef int (*func_ptr)();
static struct multiboot_boot_header_tag* tags;

static int console_dev_drv(char* filename,int c,long pos,char wr) {
  if (wr) {
    if (c=='\f') {
      vga_clear();
    } else if (c=='\b') {
      vga_backspace();
    }
    char str[2];
    str[0]=(char)c;
    str[1]='\0';
    vga_write_string(str);
    return 0;
  } else {
    return 0;
    // return devbuf_get(kbd_buf);
  }
}

static int initrd_dev_drv(char* filename,int c,long pos,char wr) {
  if (wr) {
    return 0;
  }
  if (pos>=initrd_sz) {
    return EOF;
  }
  return (initrd[pos]&0xFF);
}

static void read_initrd(struct multiboot_boot_header_tag* tags) {

  struct multiboot_tag* tag=(struct multiboot_tag*)(tags+1);
  while (tag->type!=0) {
    switch (tag->type) {
      case MULTIBOOT_TAG_TYPE_MODULE: {
        struct multiboot_tag_module* mod=(struct multiboot_tag_module*) tag;
        initrd=malloc(sizeof(char)*(mod->mod_end-mod->mod_start));
        initrd_sz=mod->mod_end-mod->mod_start;
        memcpy(initrd,mod->mod_start+0xC0000000,mod->mod_end-mod->mod_start);
      }
    }
    tag=(struct multiboot_tag*)((char*)tag+((tag->size+7)&0xFFFFFFF8));
  }
}

static void init() {
  init_vfs();
  init_devfs();
  devfs_add(console_dev_drv,"console");
  stdout=fopen("/dev/console","w");
  stdin=fopen("/dev/console","r");
  stderr=fopen("/dev/console","w");
  read_initrd(tags);
  devfs_add(initrd_dev_drv,"initrd");
  initrd_init();
  mount("/initrd/","","initrd");
  // Detect PCI
  port_long_out(0xCF8,(1<<31));
  uint32_t word=port_long_in(0xCFC);
  port_long_out(0xCF8,(1<<31)|0x4);
  if (word!=port_long_in(0xCFC)) {
    // pci_init();
  }
  // Detect and initailize serial ports
  serial_init();
  ide_init();
  // load_parts("/dev/hda");
  init_ext2();
  mount("/","/dev/hda","ext2");
  klog("INFO","MOUNT");
  FILE* f=fopen("/file","r");
  char str[256];
  fgets(str,256,f);
  str[strlen(str)-1]='\0';
  klog("INFO","Got string %s",str);
  FILE* file=fopen("/initrd/prog.elf","r");
  elf_header header;
  fread(&header,sizeof(elf_header),1,file);
  if (header.magic!=ELF_MAGIC) {
    klog("INFO","Invalid magic number for prog.elf");
    fclose(file);
  } else {
    for (int i=0;i<header.pheader_ent_nm;i++) {
      elf_pheader pheader;
      fseek(file,(header.prog_hdr)+(header.pheader_ent_sz*i),SEEK_SET);
      fread(&pheader,sizeof(elf_pheader),1,file);
      alloc_memory_virt(((pheader.memsz)/4096)+1,(void*)pheader.vaddr);
      memset((void*)pheader.vaddr,0xAA,pheader.memsz);
      if (pheader.filesz>0) {
        fseek(file,pheader.offset,SEEK_SET);
        fread((void*)pheader.vaddr,pheader.filesz,1,file);
      }
    }
    func_ptr prog=(func_ptr)header.entry;
    prog();
  }
  // for(;;) {
  //   yield();
  // }
}

void kmain(struct multiboot_boot_header_tag* hdr) {
  tags=hdr;
  cpu_init(tags);
  text_fb_info info;
  // if (header->flags&MULTIBOOT_INFO_FRAMEBUFFER_INFO&&header->framebuffer_type==2) {
  //   info.address=(char*)(((uint32_t)header->framebuffer_addr&0xFFFFFFFF)+0xC0000000);
  //   info.width=header->framebuffer_width;
  //   info.height=header->framebuffer_height;
  // } else {
    // info.address=(char*)0xffff8000000B8000;
    info.address=(char*)0xC00B8000;
    info.width=80;
    info.height=25;
  // }
  vga_init(info);
  vga_write_string("Hello long mode world!\n");
  // check_gets();
  createTask(init);
  for (;;) {
    yield();
  }
}
