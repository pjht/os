#include <string.h>
#include "vga.h"
#include <grub/text_fb_info.h>
#include <ipc/vfs.h>
#include <elf.h>
#include <mailboxes.h>
#include <memory.h>
#include <tasking.h>
#include <stdlib.h>

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

int main(char* initrd, uint32_t initrd_sz) {
  text_fb_info info;
  info.address=map_phys((void*)0xB8000,10);
  info.width=80;
  info.height=25;
  vga_init(info);
  vga_write_string("INIT VGA\n");
  int pos=0;
  uint32_t datapos;
  tar_header tar_hdr;
  for (int i=0;;i++) {
    char* tar_hdr_ptr=(char*)&tar_hdr;
    for (size_t i=0;i<sizeof(tar_hdr);i++) {
      tar_hdr_ptr[i]=initrd[pos+i];
    }
    if (tar_hdr.filename[0]=='\0') break;
    uint32_t size=getsize(tar_hdr.size);
    pos+=512;
    if (strcmp(tar_hdr.filename,"vfs")==0) {
      vga_write_string("VFS found, loading\n");
      datapos=pos;
      break;
    } else {
      vga_write_string("VFS not found\n");
      // for(;;);
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
    vga_write_string("[INFO] Invalid magic number for vfs\n");
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
    vga_write_string("Loaded VFS into memory, creating task\n");
    createTaskCr3((void*)header.entry,cr3);
    vga_write_string("Created VFS task, creating mailbox\n");
    uint32_t box=mailbox_new(16);
    vga_write_string("Created mailbox, yielding to VFS so it can create it's mailbox.\n");
    yield();
    vga_write_string("VFS yielded back, sending test message\n");
    vfs_message* msg_data=malloc(sizeof(vfs_message));
    msg_data->type=VFS_OPEN;
    msg_data->id=1;
    strcpy(&msg_data->mode[0],"r");
    strcpy(&msg_data->path[0],"/dev/sda");
    Message msg;
    msg.from=box;
    msg.to=1;
    msg.msg=msg_data;
    msg.size=sizeof(vfs_message);
    mailbox_send_msg(&msg);
    vga_write_string("Sent test message, yielding to task\n");
    yield();
    vga_write_string("Yielded and got control, getting message\n");
    msg.msg=malloc(sizeof(vfs_message));
    mailbox_get_msg(box,&msg,sizeof(vfs_message));
    vfs_message* vfs_msg=(vfs_message*)msg.msg;
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
  }
  for(;;);
}
