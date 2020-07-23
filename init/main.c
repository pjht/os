#include <string.h>
#include <grub/text_fb_info.h>
#include <ipc/vfs.h>
#include <elf.h>
#include <memory.h>
#include <tasking.h>
#include <stdlib.h>
#include <stdio.h>
#include <initrd.h>
#include <dbg.h>
#include <vfs.h>
#include <pthread.h>

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

uint32_t getsize(const char *in) {
    uint32_t size=0;
    uint32_t j;
    uint32_t count=1;
    for (j=11;j>0;j--,count*=8) {
        size+=((in[j-1]-'0')*count);
    }
    return size;
}

uint32_t find_loc(char* name,char* initrd) {
  uint32_t pos=0;
  tar_header tar_hdr;
  for (int i=0;;i++) {
    char* tar_hdr_ptr=(char*)&tar_hdr;
    for (size_t i=0;i<sizeof(tar_hdr);i++) {
      tar_hdr_ptr[i]=initrd[pos+i];
    }
    if (tar_hdr.filename[0]=='\0') break;
    uint32_t size=getsize(tar_hdr.size);
    pos+=512;
    if (strcmp(tar_hdr.filename,name)==0) {
      return pos;
      break;
    }
    pos+=size;
    if (pos%512!=0) {
      pos+=512-(pos%512);
    }
  }
  return 0;
}

char load_proc(uint32_t datapos,char* initrd) {
  int pos=0;
  elf_header header;
  pos=datapos;
  char str[256];
  int_to_ascii(pos,str);
  serial_print("POS:");
  serial_print(str);
  serial_print("\n");
  char* hdr_ptr=(char*)&header;
  for (size_t i=0;i<sizeof(elf_header);i++) {
    hdr_ptr[i]=initrd[pos];
    pos++;
  }
  if (header.magic!=ELF_MAGIC) {
    return 0;
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
    createProcCr3((void*)header.entry,cr3);
  }
  return 1;
}

// char load_proc_devfs(uint32_t datapos) {
//   serial_print("load_proc_devfs\n");
//   FILE* initrd=fopen("/dev/initrd","r");
//   elf_header header;
//   fseek(initrd,datapos,SEEK_SET);
//   fread(&header,sizeof(elf_header),1,initrd);
//   if (header.magic!=ELF_MAGIC) {
//     serial_print("Bad magic number\n");
//     return 0;
//   } else {
//     void* cr3=new_address_space();
//     for (int i=0;i<header.pheader_ent_nm;i++) {
//       elf_pheader pheader;
//       fseek(initrd,(header.prog_hdr)+(header.pheader_ent_sz*i)+datapos,SEEK_SET);
//       fread(&pheader,sizeof(elf_pheader),1,initrd);
//       serial_print("pheader.memsz=");
//       char str[256];
//       hex_to_ascii(pheader.memsz,str);
//       serial_print(str);
//       serial_print("\n");
//       char* ptr=alloc_memory(((pheader.memsz)/4096)+1);
//       memset(ptr,0,pheader.memsz);
//       if (pheader.filesz>0) {
//         fseek(initrd,pheader.offset+datapos,SEEK_SET);
//         fread(ptr,sizeof(char),pheader.filesz,initrd);
//       }
//       copy_data(cr3,ptr,pheader.memsz,(void*)pheader.vaddr);
//     }
//     createProcCr3((void*)header.entry,cr3);
//   }
//   return 1;
// }

void* thread_func(void* arg) {
  for (;;) yield();
  return NULL;
}

int main() {
  serial_print("IN INIT\n");
  pthread_t thread;
  pthread_create(&thread,NULL,thread_func,NULL);
  blockThread(THREAD_BLOCKED);
  for (int i=0;i<5;i++) {
    serial_print("YIELDING\n");
    yield();
    serial_print("YIELDED\n");
  }
  serial_print("EXITING\n");
  exit(0);
  // long size=initrd_sz();
  // char* initrd=malloc(size);
  // initrd_get(initrd);
  // exit(0);
  // uint32_t datapos=find_loc("vfs",initrd);
  // load_proc(datapos,initrd);
  // yield(); // Bochs fails here
  // rescan_vfs();
  // datapos=find_loc("devfs",initrd);
  // load_proc(datapos,initrd);
  // yieldToPID(3);
  // datapos=find_loc("initrd_drv",initrd);
  // load_proc(datapos,initrd);
  // yieldToPID(4);
  // mount("","devfs","/dev/");
  // datapos=find_loc("vga_drv",initrd);
  // serial_print("Making vga process\n");
  // load_proc_devfs(datapos);
  // serial_print("Made vga process\n");
  // yieldToPID(5);
  // FILE* file;
  // do {
  //   file=fopen("/dev/vga","w");
  // } while(file==NULL);
  // do {
  //   file=fopen("/dev/vga","w");
  // } while(file==NULL);
  // datapos=find_loc("pci",initrd);
  // load_proc(datapos,initrd);
  // free(initrd);
  // yieldToPID(4);
  // fputs("FPUTS String\n",file);
  // char str[3]={0,0,0};
  // fgets(str,2,stdin);
  // char str2[3]={0,0,0};
  // fgets(str2,2,stdin);
  // printf("Printf %s,%s\n",str,str2);
}
