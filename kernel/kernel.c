// #include "../cpu/cpu_init.h"
#include "../drivers/vga.h"
// #include "../drivers/pci.h"
// #include "../drivers/serial.h"
// #include "../cpu/i386/ports.h"
// #include "vfs.h"
// #include "../fs/devfs.h"
// #include "../fs/initrd.h"
#include <grub/text_fb_info.h>
// #include <stdlib.h>
// #include <tasking.h>
// #include <string.h>
// #include <memory.h>
#include <grub/multiboot.h>
// #include "klog.h"
// #include "elf.h"
// #include <errno.h>
// #include "../drivers/ide.h"
// #include "parts.h"
// #include "../fs/ext2.h"
// #include <stdint.h>
//
// static long initrd_sz;
// static char* initrd;
// static multiboot_info_t* mbd;
// typedef int (*func_ptr)();
//
// static int console_dev_drv(char* filename,int c,long pos,char wr) {
//   if (wr) {
//     if (c=='\f') {
//       vga_clear();
//     } else if (c=='\b') {
//       vga_backspace();
//     }
//     char str[2];
//     str[0]=(char)c;
//     str[1]='\0';
//     vga_write_string(str);
//     return 0;
//   } else {
//     return 0;
//     // return devbuf_get(kbd_buf);
//   }
// }
//
// static int initrd_dev_drv(char* filename,int c,long pos,char wr) {
//   if (wr) {
//     return 0;
//   }
//   if (pos>=initrd_sz) {
//     return EOF;
//   }
//   return initrd[pos];
// }
//
// static void read_initrd(multiboot_info_t* mbd) {
//   if ((mbd->flags&MULTIBOOT_INFO_MODS)!=0) {
//     uint32_t mods_count=mbd->mods_count;
//     if (mods_count>0) {
//       while (mods_count>0) {
//         multiboot_module_t* mods_addr=(multiboot_module_t*)(mbd->mods_addr+0xC0000000);
//         if (strcmp((char*)(mods_addr->cmdline+0xC0000000),"initrd")==0) {
//           initrd=malloc(sizeof(char)*(mods_addr->mod_end-mods_addr->mod_start));
//           initrd_sz=(mods_addr->mod_end-mods_addr->mod_start);
//           memcpy(initrd,(void*)mods_addr->mod_start+0xC0000000,initrd_sz);
//         };
//         mods_count--;
//       }
//     }
//   } else {
//     klog("PANIC","Cannnot load initrd. No modules found!");
//     for(;;) {}
//   }
//   if (!initrd) {
//     klog("PANIC","Cannnot load initrd. Initrd module not found!");
//     for(;;) {}
//   }
// }
//
// static void init() {
//   init_vfs();
//   init_devfs();
//   devfs_add(console_dev_drv,"console");
//   stdout=fopen("/dev/console","w");
//   stdin=fopen("/dev/console","r");
//   stderr=fopen("/dev/console","w");
//   // read_initrd(mbd);
//   // devfs_add(initrd_dev_drv,"initrd");
//   // initrd_init();
//   mount("/initrd/","","initrd");
//   // Detect PCI
//   port_long_out(0xCF8,(1<<31));
//   uint32_t word=port_long_in(0xCFC);
//   port_long_out(0xCF8,(1<<31)|0x4);
//   if (word!=port_long_in(0xCFC)) {
//     // pci_init();
//   }
//   // Detect and initailize serial ports
//   serial_init();
//   FILE* file=fopen("/initrd/prog.elf","r");
//   elf_header header;
//   fread(&header,sizeof(elf_header),1,file);
//   if (header.magic!=ELF_MAGIC) {
//     klog("INFO","Invalid magic number for prog.elf");
//     fclose(file);
//   } else {
//     fseek(file,header.prog_hdr,SEEK_SET);
//     elf_pheader pheader;
//     fread(&pheader,sizeof(elf_pheader),1,file);
//     alloc_memory_virt(1,(void*)pheader.vaddr);
//     fseek(file,pheader.offset,SEEK_SET);
//     fread((void*)pheader.vaddr,pheader.filesz,1,file);
//     klog("INFO","VADDR:%x",pheader.vaddr);
//     func_ptr prog=(func_ptr)header.entry;
//     int val=prog();
//     klog("INFO","RAN PROG:%d",val);
//   }
//   ide_init();
//   load_parts("/dev/hda");
//   init_ext2();
//   mount("/","/dev/hda1","ext2");
//   klog("INFO","MOUNT");
//   FILE* f=fopen("/file","r");
//   char str[256];
//   fgets(str,256,f);
//   str[strlen(str)-1]='\0';
//   klog("INFO","Got string %s",str);
//   for(;;) {
//     yield();
//   }
// }
//
//
// void func() {
//   for (;;);
// }

void kmain(multiboot_info_t* header) {
  // mbd=header;
  // cpu_init(mbd);
  text_fb_info info;
  // if (header->flags&MULTIBOOT_INFO_FRAMEBUFFER_INFO&&header->framebuffer_type==2) {
  //   info.address=(char*)(((uint32_t)header->framebuffer_addr&0xFFFFFFFF)+0xC0000000);
  //   info.width=header->framebuffer_width;
  //   info.height=header->framebuffer_height;
  // } else {
    info.address=(char*)0xffff8000000B8000;
    info.width=80;
    info.height=25;
  // }
  vga_init(info);
  vga_write_string("Hello long mode world!");
  // createTask(init);
  // for (;;) {
  //   yield();
  // }
  for(;;);
}
