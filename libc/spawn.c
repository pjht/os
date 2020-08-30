#include <spawn.h>
#include <elf.h>
#include <tasking.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <dbg.h>

int posix_spawn(pid_t* pid, const char* path, const posix_spawn_file_actions_t* file_actions, const posix_spawnattr_t* attrp,
char* const argv[], char* const envp[]) {
  FILE* image=fopen((char*)path,"r");
  elf_header header;
  fread(&header,sizeof(elf_header),1,image);
  if (header.magic!=ELF_MAGIC) {
    serial_print("Bad magic number (");
    char str[32];
    hex_to_ascii(header.magic,str);
    serial_print(str);
    serial_print(")\n");
    return 0;
  } else {
    void* address_space=new_address_space();
    for (int i=0;i<header.pheader_ent_nm;i++) {
      elf_pheader pheader;
      fseek(image,(header.prog_hdr)+(header.pheader_ent_sz*i),SEEK_SET);
      fread(&pheader,sizeof(elf_pheader),1,image);
      char* ptr=alloc_memory(((pheader.memsz)/4096)+1);
      memset(ptr,0,pheader.memsz);
      if (pheader.filesz>0) {
        fseek(image,pheader.offset,SEEK_SET);
        fread(ptr,sizeof(char),pheader.filesz,image);
      }
      copy_data(address_space,ptr,pheader.memsz,(void*)pheader.vaddr);
    }
    create_proc((void*)header.entry,address_space,NULL,NULL);
  }
  return 1;
}
