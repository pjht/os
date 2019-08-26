#include <string.h>
#include "vga.h"
#include <grub/text_fb_info.h>
#include <ipc/vfs.h>
#include <elf.h>
#include <mailboxes.h>
#include <memory.h>
#include <tasking.h>
#include <stdlib.h>
#include <stdio.h>
#include <initrd.h>

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

void display_msg(vfs_message* vfs_msg) {
  vga_write_string("Message of type ");
  char str[256];
  str[0]='\0';
  int_to_ascii(vfs_msg->type,str);
  vga_write_string(str);
  vga_write_string("\n");
  vga_write_string("ID ");
  str[0]='\0';
  int_to_ascii(vfs_msg->id,str);
  vga_write_string(str);
  vga_write_string("\n");
  vga_write_string("Mode ");
  vga_write_string(&vfs_msg->mode[0]);
  vga_write_string("\n");
  vga_write_string("FD ");
  str[0]='\0';
  int_to_ascii(vfs_msg->fd,str);
  vga_write_string(str);
  vga_write_string("\n");
  vga_write_string("Path ");
  vga_write_string(&vfs_msg->path[0]);
  vga_write_string("\n");
  vga_write_string("Flags ");
  str[0]='\0';
  hex_to_ascii(vfs_msg->flags,str);
  vga_write_string(str);
  vga_write_string("\n");
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

char load_task(uint32_t datapos,char* initrd) {
  int pos=0;
  elf_header header;
  pos=datapos;
  char* hdr_ptr=(char*)&header;
  for (size_t i=0;i<sizeof(elf_header);i++) {
    hdr_ptr[i]=initrd[pos];
    pos++;
  }
  if (header.magic!=ELF_MAGIC) {
    vga_write_string("[INFO] Invalid magic number\n");
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
    createTaskCr3((void*)header.entry,cr3);
  }
  return 1;
}

vfs_message* make_msg(vfs_message_type type, char* mode, char* path) {
  static uint32_t id=0;
  vfs_message* msg_data=malloc(sizeof(vfs_message));
  msg_data->type=type;
  msg_data->id=id;
  id++;
  strcpy(&msg_data->mode[0],mode);
  strcpy(&msg_data->path[0],path);
  return msg_data;
}

void test_vfs(char* path,uint32_t box,uint32_t fs_box) {
  vga_write_string("Sending test message\n");
  vfs_message* msg_data=make_msg(VFS_OPEN,"r",path);
  Message msg;
  msg.from=box;
  msg.to=1;
  msg.msg=msg_data;
  msg.size=sizeof(vfs_message);
  mailbox_send_msg(&msg);
  free(msg.msg);
  yield();
  vga_write_string("Getting fs_box message\n");
  yield();
  vga_write_string("Getting message\n");
  msg.msg=malloc(sizeof(vfs_message));
  mailbox_get_msg(box,&msg,sizeof(vfs_message));
  if (msg.from==0) {
    vga_write_string("No message\n");
  } else {
    vfs_message* vfs_msg=(vfs_message*)msg.msg;
    display_msg(vfs_msg);
  }
  free(msg.msg);
}

int main() {
  text_fb_info info;
  info.address=map_phys((void*)0xB8000,10);
  info.width=80;
  info.height=25;
  vga_init(info);
  vga_write_string("INIT\n");
  long size=initrd_sz();
  char* initrd=malloc(size);
  initrd_get(initrd);
  uint32_t datapos=find_loc("vfs",initrd);
  load_task(datapos,initrd);
  yield(); // Bochs fails here
  datapos=find_loc("fsdrv",initrd);
  load_task(datapos,initrd);
  free(initrd);
  yieldToPID(3);
  FILE* file;
  do {
    vga_write_string("CALLING FOPEN\n");
    file=fopen("/dev/sda","w");
    vga_write_string("FOPEN RETURNED\n");
  } while(file==NULL);
  vga_write_string("CALLING FPUTS\n");
  fputs("FPUTS String",file);
  vga_write_string("FPUTS RETURNED\n");
}
