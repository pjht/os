#include "../libc/string.h"
#include "vga.h"
#include <grub/text_fb_info.h>

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
  info.address=map_phys(0xB8000,10);
  info.width=80;
  info.height=25;
  vga_init(info);
  vga_write_string("INIT VGA\n");
  int pos=0;
  tar_header hdr;
  for (int i=0;;i++) {
    char* hdr_ptr=(char*)&hdr;
    for (size_t i=0;i<sizeof(tar_header);i++) {
      hdr_ptr[i]=initrd[pos+i];
    }
    if (hdr.filename[0]=='\0') break;
    uint32_t size=getsize(hdr.size);
    pos+=512;
    if (strcmp(hdr.filename,"init")==0) {
      vga_write_string("Init found");
    }
    pos+=size;
  }
  for (;;);
}
