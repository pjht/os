#include <dbg.h>
#include <elf.h>
#include <initrd.h>
#include <memory.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tasking.h>
#include <rpc.h>
#include <serdes.h>

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

size_t getsize(const char *in) {
    size_t size=0;
    size_t j;
    size_t count=1;
    for (j=11;j>0;j--,count*=8) {
        size+=((in[j-1]-'0')*count);
    }
    return size;
}

size_t find_loc(char* name,char* initrd) {
  size_t pos=0;
  tar_header tar_hdr;
  for (int i=0;;i++) {
    char* tar_hdr_ptr=(char*)&tar_hdr;
    for (size_t i=0;i<sizeof(tar_hdr);i++) {
      tar_hdr_ptr[i]=initrd[pos+i];
    }
    if (tar_hdr.filename[0]=='\0') break;
    size_t size=getsize(tar_hdr.size);
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

char load_proc(size_t datapos,char* initrd) {
  int pos=0;
  elf_header header;
  pos=datapos;
  char* hdr_ptr=(char*)&header;
  for (size_t i=0;i<sizeof(elf_header);i++) {
    hdr_ptr[i]=initrd[pos];
    pos++;
  }
  if (header.magic!=ELF_MAGIC) {
    return 0;
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
  }
  return 1;
}

char load_proc_devfs(size_t datapos) {
  serial_print("load_proc_devfs\n");
  FILE* initrd=fopen("/dev/initrd","r");
  elf_header header;
  fseek(initrd,datapos,SEEK_SET);
  serial_print("HERE1\n");
  fread(&header,sizeof(elf_header),1,initrd);
  if (header.magic!=ELF_MAGIC) {
    serial_print("Bad magic number (");
    char str[32];
    hex_to_ascii(header.magic,str);
    serial_print(str);
    serial_print(")\n");
    return 0;
  } else {
    serial_print("HERE2\n");
    void* address_space=new_address_space();
    for (int i=0;i<header.pheader_ent_nm;i++) {
      elf_pheader pheader;
      fseek(initrd,(header.prog_hdr)+(header.pheader_ent_sz*i)+datapos,SEEK_SET);
      fread(&pheader,sizeof(elf_pheader),1,initrd);
      serial_print("pheader.memsz=");
      char str[256];
      hex_to_ascii(pheader.memsz,str);
      serial_print(str);
      serial_print("\n");
      char* ptr=alloc_memory(((pheader.memsz)/4096)+1);
      memset(ptr,0,pheader.memsz);
      if (pheader.filesz>0) {
        fseek(initrd,pheader.offset+datapos,SEEK_SET);
        fread(ptr,sizeof(char),pheader.filesz,initrd);
      }
      copy_data(address_space,ptr,pheader.memsz,(void*)pheader.vaddr);
    }
    create_proc((void*)header.entry,address_space,NULL,NULL);
  }
  return 1;
}

int main() {
  serial_print("Init running\n");
  long size=initrd_sz();
  char* initrd=malloc(size);
  initrd_get(initrd);
  size_t datapos=find_loc("vfs",initrd);
  load_proc(datapos,initrd);
  datapos=find_loc("devfs",initrd);
  load_proc(datapos,initrd);
  int err=mount("","devfs","/dev");
  if (err) {
    serial_print("Failed to mount devfs\n");
    exit(1);
  }
  datapos=find_loc("initrd_drv",initrd);
  load_proc(datapos,initrd);
  for(int i=0;i<10000000;i++);
  serial_print("Loading VGA driver\n");
  datapos=find_loc("vga_drv",initrd);
  load_proc_devfs(datapos);
  for(int i=0;i<10000000;i++);
  serial_print("Opening /dev/vga\n");
  stdout=fopen("/dev/vga","w");
  if (!stdout) {
    serial_print("Could not open /dev/vga\n");
    exit(1);
  }
  serial_print("Writing to screen\n");
  puts("Puts test");
  printf("Printf test with file opened to %s\n","/dev/vga");
}
